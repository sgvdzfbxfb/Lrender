#ifndef TEXTURE_H
#define TEXTURE_H
#include <QString>
#include <QImage>
#include <qDebug>
#include "dataType.h"

class Texture
{
    enum
    {
        DIFFUSE,
        SPECLUAR
    };
    int imgWidth;
    int imgHeight;
    QImage texture;
public:
    QString path;
    Texture() = default;
    bool getTexture(QString path);
    Color getColorFromUv(Coord2D coord);
};

#endif // TEXTURE_H
