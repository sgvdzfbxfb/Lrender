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
#include "renderAPI.h"
#include "blinnPhongShader.h"
#include "model.h"
#include "camera.h"

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
    void setRenderColor(Color color, RenderColorType type);
    void setLightColor(Color color, LightColorType type);
    void setLightDir(Vector4D dir){renderAPI::API().shader->lightList[0].dir = dir;}
    void setRenderMode(RenderMode mode){renderAPI::API().renderMode = mode;}
    void setFaceCulling(bool val){renderAPI::API().faceCulling = val;}
    void setMultiThread(bool val){renderAPI::API().multiThread = val;}
    void saveImage(QString path){renderAPI::API().saveImage(path);}
    void loadModel(QStringList paths);
    void initDevice();
    Camera camera;
protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
signals:
    void sendModelData(int triangleCount, int vertexCount);
public slots:
    void render();

private:
    int w;
    int h;
    QTimer timer;
    void processInput();
    void resetCamera();
    void initUI();
    Ui::LRenderWidget *ui;
    Model* model;
};

#endif // LRENDERWIDGET_H
