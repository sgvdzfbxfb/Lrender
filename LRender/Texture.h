#ifndef TEXTURE_H
#define TEXTURE_H
#include <QString>
#include <QImage>
#include <qDebug>
#include "BasicDataStructure.hpp"

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
    bool LoadFromImage(QString path);
    Color Sample2D(Coord2D coord);
};

#endif // TEXTURE_H
