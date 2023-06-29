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
        renderAPI::API().shader->lightList[0].diffuse = color;
        break;
    case SPECULAR:
        renderAPI::API().shader->lightList[0].specular = color;
        break;
    case AMBIENT:
        renderAPI::API().shader->lightList[0].ambient = color;
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
    renderAPI::API().shader->lightList.push_back(Light());
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
    renderAPI::API().shader->modelMat = modelMatrix;
    renderAPI::API().shader->viewMat = camera.getViewMatrix();
    renderAPI::API().shader->projectionMat = camera.getProjectionMatrix();
    renderAPI::API().shader->eyePos = camera.position;
    renderAPI::API().shader->material.shininess = 150.f;
    model->modelRender();
    update();
}
