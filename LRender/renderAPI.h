#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <climits>
#include <immintrin.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/blocked_range2d.h>
#include "triangle.h"
#include "tools.h"
#include "frame.h"
#include "shader.h"
#include "skyBoxShader.h"

class Shader;
class SkyBoxShader;

class renderAPI
{
public:
    renderMode renderMode{ FACE };
    bool faceCulling{ true };
    bool multiThread{ true };
    std::vector<Triangle> faces;
    std::vector<Texture> textureList;
    std::vector<Texture> skyBoxTexture;
    std::vector<Triangle> skyBoxModel;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<SkyBoxShader> skyShader;
    Color backgroundColor = Color(0.0, 0.0, 0.0);
    Color pointColor = Color(0.0, 0.0, 0.0);
    Color lineColor = Color(0.0, 0.0, 0.0);
    renderAPI(int _w,int _h);
    void clearBuffer(){ frame.clearBuffer(backgroundColor); }
    QImage& getBuffer(){ return frame.getImage(); }
    bool saveImage(QString path){ return frame.saveImage(path); }
    void render(bool ifAnimation);
    void renderSkyBox();
    static void init(int w, int h)
    {
        API(w, h);
    }
    static renderAPI& API(int w = 0, int h = 0)
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
    void rasterization(Triangle& tri, bool ifAnimation);
    void facesRender(Triangle& tri);
    void skyBoxFacesRender(Triangle& tri, int faceId);
    void wireframeRedner(Triangle& tri);
    void pointsRender(Triangle &tri);
    void drawLine(Line& line);
    void convertToScreen(Triangle& tri);
    void perspectiveTrans(Triangle& tri);
    CoordI4D computeBoundingBox(Triangle& tri);
    Vector3D computeBarycentric(Triangle& pts, CoordI2D P);
    std::vector<Triangle> faceClip(Triangle& tri);
    std::optional<Line> lineClip(Line& line);
};
