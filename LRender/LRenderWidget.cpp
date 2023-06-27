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
glm::mat4 ModelMatrix;

LRenderWidget::LRenderWidget(QWidget *parent) :
    QWidget(parent),camera((float)DEFAULT_WIDTH/DEFAULT_HEIGHT, FIXED_CAMERA_FAR),
    w(DEFAULT_WIDTH), h(DEFAULT_HEIGHT), ui(new Ui::LRenderWidget), model(nullptr)
{
    ui->setupUi(this);
    setFixedSize(w, h);
    ui->FPSLabel->setStyleSheet("background:transparent");
    ui->FPSLabel->setVisible(false);
    InitDevice();
    // set render frequency
    connect(&timer,&QTimer::timeout,this,&LRenderWidget::Render);
    timer.start(1);
}

LRenderWidget::~LRenderWidget()
{
    delete ui;
    delete model;
}

void LRenderWidget::ResetCamera()
{
    ui->FPSLabel->setVisible(true);
    camera.SetModel(model->centre,model->GetYRange());
    ModelMatrix = glm::mat4(1.0f);
}

void LRenderWidget::SetRenderColor(Color color, RenderColorType type)
{
    switch(type)
    {
    case BACKGROUND:
        renderAPI::GetInstance().clearColor = color;
        break;
    case LINE:
        renderAPI::GetInstance().lineColor = color;
        break;
    case POINT:
        renderAPI::GetInstance().pointColor = color;
    }
}

void LRenderWidget::SetLightColor(Color color, LightColorType type)
{
    switch(type)
    {
    case DIFFUSE:
        renderAPI::GetInstance().shader->lightList[0].diffuse = color;
        break;
    case SPECULAR:
        renderAPI::GetInstance().shader->lightList[0].specular = color;
        break;
    case AMBIENT:
        renderAPI::GetInstance().shader->lightList[0].ambient = color;
        break;
    }
}

void LRenderWidget::LoadModel(QStringList paths)
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
    emit SendModelData(model->triangleCount,model->vertexCount);
    ResetCamera();
}

void LRenderWidget::InitDevice()
{
    renderAPI::Init(w,h);
    renderAPI::GetInstance().shader = std::make_unique<BlinnPhongShader>();
    renderAPI::GetInstance().shader->lightList.push_back(Light());
}

void LRenderWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0, 0, renderAPI::GetInstance().GetBuffer());
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

void LRenderWidget::ProcessInput()
{
    if((currentBtns & Qt::LeftButton) || (currentBtns & Qt::RightButton))
    {
        if(!lastPos.isNull())
        {
            Vector2D motion = {(float)(currentPos - lastPos).x(),(float)(currentPos - lastPos).y()};
            motion.x = (motion.x / w);
            motion.y = (motion.y / h);
            if(currentBtns & Qt::LeftButton)
            {
                camera.RotateAroundTarget(motion);
            }
            if(currentBtns & Qt::RightButton)
            {
                camera.MoveTarget(motion);
            }
        }
        lastPos = currentPos;
    }
    if(ratio != 0)
    {
        camera.CloseToTarget(ratio);
        ratio = 0;
    }
}

void LRenderWidget::Render()
{
    renderAPI::GetInstance().ClearBuffer();
    if(model == nullptr) return;
    int nowTime = QTime::currentTime().msecsSinceStartOfDay();
    if(lastFrameTime != 0)
    {
        deltaTime = nowTime - lastFrameTime;
        ui->FPSLabel->setText(QStringLiteral("FPS : ")+QString::number(1000.0 / deltaTime, 'f', 0));
    }
    lastFrameTime = nowTime;
    ProcessInput();
    renderAPI::GetInstance().shader->Model = ModelMatrix;
    renderAPI::GetInstance().shader->View = camera.GetViewMatrix();
    renderAPI::GetInstance().shader->Projection = camera.GetProjectionMatrix();
    renderAPI::GetInstance().shader->eyePos = camera.position;
    renderAPI::GetInstance().shader->material.shininess = 150.f;
    model->Draw();
    update();
}
