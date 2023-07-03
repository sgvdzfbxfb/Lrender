#pragma once

#include <QMainWindow>
#include <QPalette>
#include <QColorDialog>
#include "LRenderWidget.h"
#include "ui_LRender.h"

namespace Ui {
    class LRender;
}

class LRender : public QMainWindow
{
    Q_OBJECT

public:

    enum Option { MUTITHREAD, FACECULLING, SKYBOX };
    explicit LRender(QWidget *parent = nullptr);
    ~LRender();
    void setOption(Option option, bool val);
    void setLightColor(lightColorType type, QColor color);
    void setCameraPara(cameraPara para, float val);
    void setLightDir();
private slots:
    void on_actionopen_file_triggered();

    void on_actionsave_image_triggered();

    void on_LineCheckBox_clicked();

    void on_VertexCheckBox_clicked();

    void on_actionMultiThread_triggered();

    void on_actionFaceCulling_triggered();

    void on_actionSkyBox_triggered();

    void on_FovSilder_valueChanged(int value);

    void on_NearSilder_valueChanged(int value);

    void on_SpecularColorSet_clicked();

    void on_DiffuseColorSet_clicked();

    void on_AmbientColorSet_clicked();

    void on_PitchSlider_valueChanged(int value);

    void on_YawDial_valueChanged(int value);

private:
    Ui::LRender *ui;
    void initUI();
    void initSignalAndSlot();
    QColor specularColor;
    QColor diffuseColor;
    QColor ambientColor;
};
