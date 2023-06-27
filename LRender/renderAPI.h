#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <climits>
#include <bitset>
#include <immintrin.h>
#include "dataType.h"
#include "frame.h"
#include "texture.h"
#include "shader.h"

class Shader;

class renderAPI
{
public:
    RenderMode renderMode{ FACE };
    bool faceCulling{ true };
    bool multiThread{ true };
    std::vector<Vertex> vertexList;
    std::vector<unsigned> indices;
    std::vector<Texture> textureList;
    std::unique_ptr<Shader> shader;
    Color clearColor;
    Color pointColor;
    Color lineColor;
    renderAPI(int _w,int _h);
    void clearBuffer(){ frame.clearBuffer(clearColor);}
    QImage& getBuffer(){return frame.getImage();}
    bool saveImage(QString path){return frame.saveImage(path);}
    void render();
    static void init(int w,int h)
    {
        API(w, h);
    }
    static renderAPI& API(int w = 0 ,int h = 0)
    {
        static renderAPI renderWork(w, h);
        return renderWork;
    }

    renderAPI(const renderAPI&) = delete;
    renderAPI(renderAPI &&) = delete;
    renderAPI& operator=(const renderAPI&) = delete;
    renderAPI& operator=(renderAPI&&) = delete;

private:
    int width;
    int height;
    std::array<BorderPlane, 6> viewBox;
    std::array<BorderLine, 4> screenEdge;
    Frame frame;
    void rasterization(Triangle& tri);
    void facesRender(Triangle& tri);
    void wireframeRedner(Triangle& tri);
    void pointsRender(Triangle &tri);
    void drawLine(Line& line);
    void convertToScreen(Triangle &tri);
    void perspectiveTrans(Triangle& tri);
    CoordI4D computeBoundingBox(Triangle & tri);
    Vector3D computeBarycentric(Triangle& pts, CoordI2D P);
    std::vector<Triangle> faceClip(Triangle& tri);
    std::optional<Line> lineClip(Line& line);
};
