#include "texture.h"
#include <QDebug>

bool Texture::getTexture(QString path)
{
    this->path = path;
    if(texture.load(path))
    {
        texture = texture.mirrored();
        imgWidth = texture.width();
        imgHeight = texture.height();
        return true;
    }
    return false;
}

Color Texture::getColorFromUv(Coord2D coord)
{
    int x = static_cast<int>(coord.x * imgWidth - 0.5f) % imgWidth;
    int y = static_cast<int>(coord.y * imgHeight - 0.5f) % imgHeight;
    x = x < 0 ? imgWidth + x : x;
    y = y < 0 ? imgHeight + y : y;
    return Color(texture.pixelColor(x, y).red() / 255.f, texture.pixelColor(x, y).green() / 255.f, texture.pixelColor(x, y).blue() / 255.f);
}
