#ifndef LRENDERWIDGET_H
#define LRENDERWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QFile>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QColor>
#include <QImage>
#include <QPixmap>
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>
#include "src/SRendererDevice.h"
#include "BlinnPhongShader.h"
#include "Model.h"
#include "Camera.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define FIXED_CAMERA_FAR 100.f

namespace Ui {
class LRenderWidget;
}

class LRenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LRenderWidget(QWidget *parent = nullptr);
    ~LRenderWidget();
    void SetRenderColor(Color color, RenderColorType type);
    void SetLightColor(Color color, LightColorType type);
    void SetLightDir(Vector4D dir){SRendererDevice::GetInstance().shader->lightList[0].dir = dir;}
    void SetRenderMode(RenderMode mode){SRendererDevice::GetInstance().renderMode = mode;}
    void SetFaceCulling(bool val){SRendererDevice::GetInstance().faceCulling = val;}
    void SetMultiThread(bool val){SRendererDevice::GetInstance().multiThread = val;}
    void SaveImage(QString path){SRendererDevice::GetInstance().SaveImage(path);}
    void LoadModel(QString path);
    void InitDevice();
    Camera camera;
protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
signals:
    void SendModelData(int triangleCount, int vertexCount);
public slots:
    void Render();

private:
    int w;
    int h;
    QTimer timer;
    void ProcessInput();
    void ResetCamera();
    void InitUI();
    Ui::LRenderWidget *ui;
    Model* model;
};

#endif // LRENDERWIDGET_H
