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
    int w;
    int h;
    QImage texture;
public:
    QString path;
    Texture() = default;
    bool loadFromImage(QString path);
    Color sample2D(Coord2D coord);
};

#endif // TEXTURE_H
