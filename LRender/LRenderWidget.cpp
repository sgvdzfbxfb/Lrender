#include "LRenderWidget.h"
#include "ui_LRenderWidget.h"
#include <QDebug>
#include <chrono>

int lastFrameTime;
int deltaTime;
QPoint lastPos;
QPoint currentPos;
int ratio;
Qt::MouseButtons currentBtns;
glm::mat4 modelMatrix;

LRenderWidget::LRenderWidget(QWidget *parent) :
    QWidget(parent),camera((float)DEFAULT_WIDTH/DEFAULT_HEIGHT, FIXED_CAMERA_FAR),
    skyBoxCamera((float)DEFAULT_WIDTH / DEFAULT_HEIGHT, FIXED_CAMERA_FAR),
    scWidth(DEFAULT_WIDTH), scHeight(DEFAULT_HEIGHT), ui(new Ui::LRenderWidget), model(nullptr)
{
    ui->setupUi(this);
    setFixedSize(scWidth, scHeight);
    ui->FPSLabel->setStyleSheet("background:transparent");
    ui->FPSLabel->setVisible(false);
    initDevice();
    // set render frequency
    connect(&timer, &QTimer::timeout, this, &LRenderWidget::render);
    timer.start(1);
    this->grabKeyboard();
}

LRenderWidget::~LRenderWidget()
{
    delete ui;
    delete model;
}

void LRenderWidget::resetCamera()
{
    ui->FPSLabel->setVisible(true);
    skyBoxCamera.setModel(Coord3D(0.0, 0.0, 0.0), 1.0);
    camera.setModel(model->modelCenter, model->getYRange());
    modelMatrix = glm::mat4(1.0f);
}

void LRenderWidget::setFigureColor(Color color, renderFigure type)
{
    switch(type)
    {
    case BACKGROUND:
        renderAPI::API().backgroundColor = color;
        break;
    case LINE:
        renderAPI::API().lineColor = color;
        break;
    case POINT:
        renderAPI::API().pointColor = color;
    }
}

void LRenderWidget::setLightColor(Color color, lightColorType type)
{
    switch(type)
    {
    case DIFFUSE:
        renderAPI::API().shader->lightList.at(0).diffuse = color;
        break;
    case SPECULAR:
        renderAPI::API().shader->lightList.at(0).specular = color;
        break;
    case AMBIENT:
        renderAPI::API().shader->lightList.at(0).ambient = color;
        break;
    }
}

void LRenderWidget::loadModel(QStringList paths)
{
    Model *newModel = new Model(paths);
    if(!newModel->loadSuccess)
    {
        QMessageBox::critical(this,"Error","Model loading error!");
        delete newModel;
        return;
    }
    if(model != nullptr)
        delete model;
    model = newModel;
    emit sendModelData(model->faceNum, model->vertexNum);
    resetCamera();
}

void LRenderWidget::initDevice()
{
    renderAPI::init(scWidth,scHeight);
    renderAPI::API().shader = std::make_unique<BlinnPhongShader>();
    renderAPI::API().skyShader = std::make_unique<SkyBoxShader>();
    renderAPI::API().shader->lightList.push_back(Light());
    loadSkyBox("D:/Code/lrender/LRender/skybox/skybox1");
    renderAPI::API().skyBoxTexture = skyBoxTexture;
    renderAPI::API().skyBoxModel = skyBoxModel;
}

void LRenderWidget::switchLightMode(bool turnLight)
{
    if (turnLight) {
        QPalette dark;
        dark.setColor(QPalette::WindowText, Qt::black);
        ui->FPSLabel->setPalette(dark);
    }
    else {
        QPalette white;
        white.setColor(QPalette::WindowText, Qt::white);
        ui->FPSLabel->setPalette(white);
    }
}

void LRenderWidget::loadSkyBox(std::string skyPath)
{
    std::vector<Vertex> skyBoxVers;
    Vertex Ver1; Ver1.worldPos = Coord3D(5.0, 5.0, -5.0); skyBoxVers.push_back(Ver1);
    Vertex Ver2; Ver2.worldPos = Coord3D(5.0, -5.0, -5.0); skyBoxVers.push_back(Ver2);
    Vertex Ver3; Ver3.worldPos = Coord3D(-5.0, -5.0, -5.0); skyBoxVers.push_back(Ver3);
    Vertex Ver4; Ver4.worldPos = Coord3D(-5.0, 5.0, -5.0); skyBoxVers.push_back(Ver4);
    Vertex Ver5; Ver5.worldPos = Coord3D(5.0, 5.0, 5.0); skyBoxVers.push_back(Ver5);
    Vertex Ver6; Ver6.worldPos = Coord3D(5.0, -5.0, 5.0); skyBoxVers.push_back(Ver6);
    Vertex Ver7; Ver7.worldPos = Coord3D(-5.0, -5.0, 5.0); skyBoxVers.push_back(Ver7);
    Vertex Ver8; Ver8.worldPos = Coord3D(-5.0, 5.0, 5.0); skyBoxVers.push_back(Ver8);

    Triangle face21{ skyBoxVers.at(2), skyBoxVers.at(6), skyBoxVers.at(7) };
    face21.at(0).texUv = Coord2D(0.0, 0.0); face21.at(1).texUv = Coord2D(1.0, 0.0); face21.at(2).texUv = Coord2D(1.0, 1.0);
    skyBoxModel.push_back(face21);
    Triangle face22{ skyBoxVers.at(2), skyBoxVers.at(7), skyBoxVers.at(3) };
    face22.at(0).texUv = Coord2D(0.0, 0.0); face22.at(1).texUv = Coord2D(1.0, 1.0); face22.at(2).texUv = Coord2D(0.0, 1.0);
    skyBoxModel.push_back(face22);

    Triangle face61{ skyBoxVers.at(2), skyBoxVers.at(1), skyBoxVers.at(5) };
    face61.at(0).texUv = Coord2D(0.0, 0.0); face61.at(1).texUv = Coord2D(1.0, 0.0); face61.at(2).texUv = Coord2D(1.0, 1.0);
    skyBoxModel.push_back(face61);
    Triangle face62{ skyBoxVers.at(2), skyBoxVers.at(5), skyBoxVers.at(6) };
    face62.at(0).texUv = Coord2D(0.0, 0.0); face62.at(1).texUv = Coord2D(1.0, 1.0); face62.at(2).texUv = Coord2D(0.0, 1.0);
    skyBoxModel.push_back(face62);

    Triangle face11{ skyBoxVers.at(1), skyBoxVers.at(2), skyBoxVers.at(3) };
    face11.at(0).texUv = Coord2D(0.0, 0.0); face11.at(1).texUv = Coord2D(1.0, 0.0); face11.at(2).texUv = Coord2D(1.0, 1.0);
    skyBoxModel.push_back(face11);
    Triangle face12{ skyBoxVers.at(1), skyBoxVers.at(3), skyBoxVers.at(0) };
    face12.at(0).texUv = Coord2D(0.0, 0.0); face12.at(1).texUv = Coord2D(1.0, 1.0); face12.at(2).texUv = Coord2D(0.0, 1.0);
    skyBoxModel.push_back(face12);

    Triangle face41{ skyBoxVers.at(5), skyBoxVers.at(1), skyBoxVers.at(0) };
    face41.at(0).texUv = Coord2D(0.0, 0.0); face41.at(1).texUv = Coord2D(1.0, 0.0); face41.at(2).texUv = Coord2D(1.0, 1.0);
    skyBoxModel.push_back(face41);
    Triangle face42{ skyBoxVers.at(5), skyBoxVers.at(0), skyBoxVers.at(4) };
    face42.at(0).texUv = Coord2D(0.0, 0.0); face42.at(1).texUv = Coord2D(1.0, 1.0); face42.at(2).texUv = Coord2D(0.0, 1.0);
    skyBoxModel.push_back(face42);

    Triangle face51{ skyBoxVers.at(7), skyBoxVers.at(4), skyBoxVers.at(0) };
    face51.at(0).texUv = Coord2D(0.0, 0.0); face51.at(1).texUv = Coord2D(1.0, 0.0); face51.at(2).texUv = Coord2D(1.0, 1.0);
    skyBoxModel.push_back(face51);
    Triangle face52{ skyBoxVers.at(7), skyBoxVers.at(0), skyBoxVers.at(3) };
    face52.at(0).texUv = Coord2D(0.0, 0.0); face52.at(1).texUv = Coord2D(1.0, 1.0); face52.at(2).texUv = Coord2D(0.0, 1.0);
    skyBoxModel.push_back(face52);

    Triangle face31{ skyBoxVers.at(6), skyBoxVers.at(5), skyBoxVers.at(4) };
    face31.at(0).texUv = Coord2D(0.0, 0.0); face31.at(1).texUv = Coord2D(1.0, 0.0); face31.at(2).texUv = Coord2D(1.0, 1.0);
    skyBoxModel.push_back(face31);
    Triangle face32{ skyBoxVers.at(6), skyBoxVers.at(4), skyBoxVers.at(7) };
    face32.at(0).texUv = Coord2D(0.0, 0.0); face32.at(1).texUv = Coord2D(1.0, 1.0); face32.at(2).texUv = Coord2D(0.0, 1.0);
    skyBoxModel.push_back(face32);

    std::vector<std::string> skyPaths;
    getAllImageFiles(skyPath, skyPaths);

    for (int i = 0; i < skyPaths.size(); ++i) {
        Texture texture;
        if (texture.getTexture(QString::fromStdString(skyPaths.at(i)))) {
            skyBoxTexture.push_back(texture);
            qDebug() << "load skybox texture img:" << QString::fromStdString(skyPaths.at(i));
        }
    }
}

void LRenderWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0, 0, renderAPI::API().getBuffer());
}

void LRenderWidget::mousePressEvent(QMouseEvent *event)
{
    currentBtns = event->buttons();
    currentPos = event->pos();
    lastPos = {0,0};
}

void LRenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    currentBtns = event->buttons();
}

void LRenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    currentPos = event->pos();
}

void LRenderWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    QPoint res;
    if (!numPixels.isNull()) {
        res = numPixels;
    } else if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        res = numSteps;
    }
    ratio += res.y();
}

void LRenderWidget::keyPressEvent(QKeyEvent* event)
{
    /*Vector3D updatePos = camera.getPositon();
    switch (event->key()) {
        case Qt::Key_A: updatePos.x -= 0.3; break;
        case Qt::Key_D: updatePos.x += 0.3; break;
        case Qt::Key_W: updatePos.z += 0.3; break;
        case Qt::Key_S: updatePos.z -= 0.3; break;
        case Qt::Key_Q: updatePos.y += 0.3; break;
        case Qt::Key_E: updatePos.y -= 0.3; break;
        default: break;
    }
    camera.setPositon(updatePos);*/
}

void LRenderWidget::processInput()
{
    if((currentBtns & Qt::LeftButton) || (currentBtns & Qt::MidButton))
    {
        if(!lastPos.isNull())
        {
            Vector2D motion = {(float)(currentPos - lastPos).x(), (float)(currentPos - lastPos).y()};
            motion.x = (motion.x / scWidth);
            motion.y = (motion.y / scHeight);
            if(currentBtns & Qt::LeftButton)
            {
                camera.rotateAroundTarget(motion);
                skyBoxCamera.rotateAroundTarget(motion);
            }
            if(currentBtns & Qt::MidButton)
            {
                camera.moveTarget(motion);
            }
        }
        lastPos = currentPos;
    }
    if(ratio != 0)
    {
        camera.closeToTarget(ratio);
        ratio = 0;
    }
}

void LRenderWidget::render()
{
    renderAPI::API().clearBuffer();
    if(model == nullptr) return;
    int nowTime = QTime::currentTime().msecsSinceStartOfDay();
    if(lastFrameTime != 0)
    {
        deltaTime = nowTime - lastFrameTime;
        ui->FPSLabel->setText(QStringLiteral("FPS : ") + QString::number(1000.0 / deltaTime, 'f', 0));
    }
    lastFrameTime = nowTime;
    processInput();
    
    // render skybox
    if (ifShowSkyBox) {
        renderAPI::API().skyShader->modelMat = modelMatrix;
        renderAPI::API().skyShader->viewMat = skyBoxCamera.getViewMatrix();
        renderAPI::API().skyShader->projectionMat = skyBoxCamera.getProjectionMatrix();
        renderAPI::API().skyShader->material.shininess = 150.f;
        renderAPI::API().skyShader->eyePos = skyBoxCamera.position;
        renderAPI::API().renderSkyBox();
    }

    // render model
    renderAPI::API().shader->modelMat = modelMatrix;
    renderAPI::API().shader->viewMat = camera.getViewMatrix();
    renderAPI::API().shader->projectionMat = camera.getProjectionMatrix();
    renderAPI::API().shader->eyePos = camera.position;
    renderAPI::API().shader->material.shininess = 150.f;
    model->modelRender();
    update();
}
