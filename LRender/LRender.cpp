#include "LRender.h"
#include "ui_LRender.h"

static inline QString generateStyleSheet(QColor color)
{
    return "background-color: rgb("
        + QString::number(static_cast<int>(color.red())) + ','
        + QString::number(static_cast<int>(color.green())) + ','
        + QString::number(static_cast<int>(color.blue())) + ");";
}

LRender::LRender(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::LRender)
{
    ui->setupUi(this);
    initUI();
    initSignalAndSlot();
}

LRender::~LRender()
{}

void LRender::setOption(Option option, bool val)
{
    if (option == MUTITHREAD)
    {
        ui->actionMultiThread->setChecked(val);
        ui->RenderWidget->setMultiThread(val);
    }
    else if (option == FACECULLING)
    {
        ui->actionFaceCulling->setChecked(val);
        ui->RenderWidget->setFaceCulling(val);
    }
    else
    {
        ui->actionSkyBox->setChecked(val);
        ui->RenderWidget->setSkyBox(val);
    }
}
void LRender::setLightColor(lightColorType type, QColor color)
{
    switch (type)
    {
    case SPECULAR:
        ui->SpecularColor->setStyleSheet(generateStyleSheet(color));
        specularColor = color;
        break;
    case DIFFUSE:
        ui->DiffuseColor->setStyleSheet(generateStyleSheet(color));
        diffuseColor = color;
        break;
    case AMBIENT:
        ui->AmbientColor->setStyleSheet(generateStyleSheet(color));
        ambientColor = color;
        break;
    }
    ui->RenderWidget->setLightColor({ color.red() / 255.f,
                                     color.green() / 255.f,
                                     color.blue() / 255.f }, type);
}
void LRender::setCameraPara(cameraPara para, float val)
{
    if (para == FOV)
    {
        ui->FovLabel->setText(QString::number(static_cast<int>(val)));
        ui->RenderWidget->camera.fov = val;
    }
    else
    {
        ui->NearLabel->setText(QString::number(val, 'f', 1));
        ui->RenderWidget->camera.zNear = val;
    }
}
void LRender::setLightDir()
{
    Vector3D lightDir;
    float pitch = glm::radians(glm::clamp(static_cast<float>(ui->PitchSlider->value()), -89.9f, 89.9f));
    float yaw = -glm::radians(static_cast<float>(ui->YawDial->value()));

    lightDir.x = (1.f * (float)std::cos(pitch) * (float)std::sin(yaw));
    lightDir.y = (1.f * (float)std::sin(pitch));
    lightDir.z = (1.f * (float)std::cos(pitch) * (float)std::cos(yaw));

    ui->RenderWidget->setLightDir(Vector4D(-lightDir, 0.f));
}

void LRender::on_actionopen_file_triggered()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "Open Model File", "", "OBJ(*.obj)");
    if (!filePaths.isEmpty()) ui->RenderWidget->loadModel(filePaths);
}

void LRender::on_actionsave_image_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG(*.png)");
    if (!fileName.isEmpty())
        ui->RenderWidget->saveImage(fileName);
}

void LRender::initUI()
{
    setFixedSize(1382, 751);
#if 0
    // light background
    ui->RenderWidget->setFigureColor({ 1.00f, 1.00f, 1.00f }, BACKGROUND);
    ui->RenderWidget->setFigureColor({ 0.098f, 0.137f, 0.176f }, LINE);
    ui->RenderWidget->setFigureColor({ 0.098f, 0.137f, 0.176f }, POINT);
    ui->RenderWidget->switchLightMode(true);
#else
    // dark background
    ui->RenderWidget->setFigureColor({ 9.f / 255.f, 12.f / 255.f, 25.f / 255.f }, BACKGROUND);
    ui->RenderWidget->setFigureColor({ 0.98f, 0.98f, 0.98f }, LINE);
    ui->RenderWidget->setFigureColor({ 0.98f, 0.98f, 0.98f }, POINT);
    ui->RenderWidget->switchLightMode(false);

#endif
    setOption(MUTITHREAD, true);
    setOption(FACECULLING, true);
    setOption(SKYBOX, false);
    setCameraPara(FOV, 60.f);
    setCameraPara(NEAR, 1.0f);
    setLightColor(SPECULAR, QColor(255, 255, 255));
    setLightColor(DIFFUSE, QColor(153, 153, 153));
    setLightColor(AMBIENT, QColor(102, 102, 102));
    setLightDir();
    QColorDialog::setCustomColor(0, QColor(255, 255, 255));
    QColorDialog::setCustomColor(2, QColor(153, 153, 153));
    QColorDialog::setCustomColor(4, QColor(102, 102, 102));
}

void LRender::initSignalAndSlot()
{
    connect(ui->RenderWidget, &LRenderWidget::sendModelData, this,
        [this](int triangleCount, int vertexCount)
        {
            ui->TriangleNumberLabel->setText(QString::number(triangleCount));
            ui->VertexNumberLabel->setText(QString::number(vertexCount));
        });
}

void LRender::on_LineCheckBox_clicked()
{
    if (ui->LineCheckBox->isChecked())
    {
        if (ui->VertexCheckBox->isChecked())
            ui->VertexCheckBox->setChecked(false);
        ui->RenderWidget->setRenderMode(EDGE);
    }
    else
    {
        ui->RenderWidget->setRenderMode(FACE);
    }
}

void LRender::on_VertexCheckBox_clicked()
{
    if (ui->VertexCheckBox->isChecked())
    {
        if (ui->LineCheckBox->isChecked())
            ui->LineCheckBox->setChecked(false);
        ui->RenderWidget->setRenderMode(VERTEX);
    }
    else
    {
        ui->RenderWidget->setRenderMode(FACE);
    }
}

void LRender::on_actionMultiThread_triggered()
{
    setOption(MUTITHREAD, ui->actionMultiThread->isChecked());
}

void LRender::on_actionFaceCulling_triggered()
{
    setOption(FACECULLING, ui->actionFaceCulling->isChecked());
}

void LRender::on_actionSkyBox_triggered()
{
    setOption(SKYBOX, ui->actionSkyBox->isChecked());
}

void LRender::on_FovSilder_valueChanged(int value)
{
    setCameraPara(FOV, static_cast<float>(value));
}

void LRender::on_NearSilder_valueChanged(int value)
{
    setCameraPara(NEAR, value / 10.f);
}

void LRender::on_SpecularColorSet_clicked()
{
    QColor color = QColorDialog::getColor(specularColor, this, "Select Specular Color");
    if (color.isValid())
        setLightColor(SPECULAR, color);
}

void LRender::on_DiffuseColorSet_clicked()
{
    QColor color = QColorDialog::getColor(diffuseColor, this, "Select Diffuse Color");
    if (color.isValid())
        setLightColor(DIFFUSE, color);
}

void LRender::on_AmbientColorSet_clicked()
{
    QColor color = QColorDialog::getColor(ambientColor, this, "Select Ambient Color");
    if (color.isValid())
        setLightColor(AMBIENT, color);
}

void LRender::on_PitchSlider_valueChanged(int value)
{
    setLightDir();
}

void LRender::on_YawDial_valueChanged(int value)
{
    setLightDir();
}