#pragma once

#include <QMainWindow>
#include <QPalette>
#include <QFileDialog>
#include <QColorDialog>
#include "LRenderWidget.h"
#include "About.h"
#include "ui_LRender.h"

namespace Ui {
    class LRender;
}

class LRender : public QMainWindow
{
    Q_OBJECT

public:

    enum Option { MUTITHREAD, FACECULLING };
    explicit LRender(QWidget *parent = nullptr);
    ~LRender();
    void SetOption(Option option, bool val);
    void SetLightColor(LightColorType type, QColor color);
    void SetCameraPara(CameraPara para, float val);
    void SetLightDir();
private slots:
    void on_actionopen_file_triggered();

    void on_actionsave_image_triggered();

    void on_LineCheckBox_clicked();

    void on_VertexCheckBox_clicked();

    void on_actionMultiThread_triggered();

    void on_actionFaceCulling_triggered();

    void on_FovSilder_valueChanged(int value);

    void on_NearSilder_valueChanged(int value);

    void on_SpecularColorSet_clicked();

    void on_DiffuseColorSet_clicked();

    void on_AmbientColorSet_clicked();

    void on_PitchSlider_valueChanged(int value);

    void on_YawDial_valueChanged(int value);

    void on_actionAbout_triggered();
private:
    Ui::LRender *ui;
    void InitUI();
    void InitSignalAndSlot();
    QColor specularColor;
    QColor diffuseColor;
    QColor ambientColor;
};
