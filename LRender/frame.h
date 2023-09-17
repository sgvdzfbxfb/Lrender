#pragma once
#include "lrenderBasicCore.h"
#include <string>
#include <cfloat>
#include <QImage>
#include <QColor>
#include <QString>

class Frame
{
public:
    Frame(int _w, int _h);
    bool updateZbuffer(int x, int y, float z);
    void setPixel(int x, int y, Color color);
    bool saveImage(QString filePath);
    void clearBuffer(Color color);
    QImage& getImage(){return colorBuffer;}
private:
	int frameWidth;
	int frameHeight;
    std::vector<float> zBuffer;
    QImage colorBuffer;
};
