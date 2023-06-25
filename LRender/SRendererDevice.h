#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <climits>
#include <bitset>
#include <immintrin.h>
#include "BasicDataStructure.hpp"
#include "FrameBuffer.h"
#include "Texture.h"
#include "Shader.hpp"

class Shader;

struct EdgeEquation
{
    VectorI3D As,Bs,Cs;
    EdgeEquation(const Triangle &tri);
    Vector3D GetBarycentric(VectorI3D val);
};

class SRendererDevice
{
public:
    RenderMode renderMode{FACE};
    bool faceCulling{true};
    bool multiThread{true};
    std::vector<Vertex> vertexList;
    std::vector<unsigned> indices;
    std::vector<Texture> textureList;
    std::unique_ptr<Shader> shader;
    Color clearColor;
    Color pointColor;
    Color lineColor;
    SRendererDevice(int _w,int _h);
    void ClearBuffer(){frameBuffer.ClearBuffer(clearColor);}
    QImage& GetBuffer(){return frameBuffer.GetImage();}
    bool SaveImage(QString path){return frameBuffer.SaveImage(path);}
    void Render();
    static void Init(int w,int h)
    {
        GetInstance(w, h);
    }
    static SRendererDevice& GetInstance(int w = 0 ,int h = 0)
    {
        static SRendererDevice Instance(w, h);
        return Instance;
    }

    SRendererDevice(const SRendererDevice&) = delete;
    SRendererDevice(SRendererDevice &&) = delete;
    SRendererDevice& operator=(const SRendererDevice&) = delete;
    SRendererDevice& operator=(SRendererDevice&&) = delete;

private:
    int w;
    int h;
    std::array<BorderPlane, 6> viewPlanes;
    std::array<BorderLine, 4> screenLines;
    FrameBuffer frameBuffer;
    void ProcessTriangle(Triangle& tri);
    void RasterizationTriangle(Triangle& tri);
    void WireframedTriangle(Triangle& tri);
    void PointedTriangle(Triangle &tri);
    void DrawLine(Line& line);
    void ConvertToScreen(Triangle &tri);
    void ExecutePerspectiveDivision(Triangle& tri);
    CoordI4D GetBoundingBox(Triangle & tri);
    Vector3D GetBarycentric(Triangle& pts, CoordI2D P);
    std::vector<Triangle> ClipTriangle(Triangle& tri);
    std::optional<Line> ClipLine(Line& line);
};
