#include "dataType.h"
#include "frame.h"
#include <iostream>
#include <QDebug>

Frame::Frame(int _w, int _h):frameWidth(_w), frameHeight(_h), zBuffer(frameWidth * frameHeight),
    colorBuffer(frameWidth, frameHeight, QImage::Format_RGB888)
{
    colorBuffer.fill(QColor(0.f,0.f,0.f));
    std::fill(zBuffer.begin(), zBuffer.end(), 1.f);
}

bool Frame::updateZbuffer(int x, int y, float z)
{
    if(z < zBuffer[y * frameWidth + x])
    {
        zBuffer[y * frameWidth + x] = z;
        return true;
    }
    return false;
}

void Frame::setPixel(int x, int y, Color color)
{
    colorBuffer.setPixelColor(x, frameHeight - 1 - y
                ,QColor(color.r * 255.f, color.g * 255.f, color.b * 255.f));
}

void Frame::clearBuffer(Color color)
{
    std::fill(zBuffer.begin(), zBuffer.end(), 1.f);
    colorBuffer.fill(QColor(color.x * 255.f, color.y * 255.f, color.z * 255.f));
}

bool Frame::saveImage(QString filePath)
{
    return colorBuffer.save(filePath);
}
