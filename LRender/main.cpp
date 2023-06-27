#include "LRender.h"
#define GLM_FORCE_AVX2
#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    QApplication qtApp(argc, argv);
    LRender lrender;
    lrender.show();
    return qtApp.exec();
}
