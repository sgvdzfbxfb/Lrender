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
#include <QKeyEvent>
#include <QMessageBox>
#include <QFileDialog>
#include "renderAPI.h"
#include "blinnPhongShader.h"
#include "skyBoxShader.h"
#include "model.h"
#include "camera.h"

#define DEFAULT_WIDTH 1000
#define DEFAULT_HEIGHT 700
#define FIXED_CAMERA_FAR 300.f

namespace Ui {
class LRenderWidget;
}

class LRenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LRenderWidget(QWidget *parent = nullptr);
    ~LRenderWidget();
    void setFigureColor(Color color, renderFigure type);
    void setLightColor(Color color, lightColorType type);
    void setLightDir(Vector4D dir){renderAPI::API().shader->lightList.at(0).dir = dir;}
    void setRenderMode(renderMode mode){renderAPI::API().renderMode = mode;}
    void setFaceCulling(bool val){renderAPI::API().faceCulling = val;}
    void setMultiThread(bool val){renderAPI::API().multiThread = val;}
    void saveImage(QString path){renderAPI::API().saveImage(path);}
    void loadModel(QStringList paths);
    void initDevice();
    void switchLightMode(bool turnLight);
    void loadSkyBox(std::string skyPath);
    Camera camera;
    Camera skyBoxCamera;
protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
signals:
    void sendModelData(int triangleCount, int vertexCount);
public slots:
    void render();

private:
    int scWidth;
    int scHeight;
    QTimer timer;
    void processInput();
    void resetCamera();
    Ui::LRenderWidget *ui;
    Model* model;
    std::vector<Texture> skyBoxTexture;
    std::vector<Triangle> skyBoxModel;
};

#endif // LRENDERWIDGET_H
