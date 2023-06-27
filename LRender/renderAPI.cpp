﻿#include "renderAPI.h"
#include <QDebug>
#include <QTime>

// helper functions
static inline bool judgeInsideTriangle(Vector3D bc_screen)
{
    bool flag = true;
    if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) flag = false;
    return flag;
}

template<class T>
static inline T calculateInterpolation(T a, T b, float alpha)
{
    return a * (1 - alpha) + b * alpha;
}

static inline CoordI2D calculateInterpolation(CoordI2D a, CoordI2D b, float alpha)
{
    CoordI2D res;
    res.x = static_cast<int>(a.x * (1 - alpha) + b.x * alpha + 0.5f);
    res.y = static_cast<int>(a.y * (1 - alpha) + b.y * alpha + 0.5f);
    return res;
}

static inline Vertex calculateInterpolation(Vertex a, Vertex b, float alpha)
{
    Vertex res;
    res.clipPos = calculateInterpolation(a.clipPos, b.clipPos, alpha);
    res.worldPos = calculateInterpolation(a.worldPos, b.worldPos, alpha);
    res.normal = calculateInterpolation(a.normal, b.normal, alpha);
    res.texUv = calculateInterpolation(a.texUv, b.texUv, alpha);
    return res;
}

template<class T>
static inline float calculateDistance(T point, T border)
{
    return glm::dot(point, border);
}

template<class T,size_t N>
static inline std::bitset<N> getClipCode(T point, std::array<T, N>& clip)
{
    std::bitset<N> res;
    for(int i = 0; i < N; i++)
        if(calculateDistance(point, clip[i]) < 0) res.set(i, 1);
    return res;
}

void renderAPI::perspectiveTrans(Triangle &tri)
{
    for (int i = 0; i < 3; i++)
    {
        tri[i].clipPos.x /= tri[i].clipPos.w;
        tri[i].clipPos.y /= tri[i].clipPos.w;
        tri[i].clipPos.z /= tri[i].clipPos.w;
    }
}

void renderAPI::convertToScreen(Triangle &tri)
{
    for (int i = 0; i < 3; i++)
    {
        tri[i].screenPos.x = static_cast<int>(0.5f * width * (tri[i].clipPos.x + 1.0f) + 0.5f);
        tri[i].screenPos.y = static_cast<int>(0.5f * height * (tri[i].clipPos.y + 1.0f) + 0.5f);
        tri[i].zValue = tri[i].clipPos.z;
    }
}

std::vector<Triangle> constructTriangle(std::vector<Vertex> vertexList)
{
    std::vector<Triangle> res;
    for(int i = 0; i < vertexList.size() - 2; i++)
    {
        int k = (i + 1) % vertexList.size();
        int m = (i + 2) % vertexList.size();
        Triangle tri{vertexList[0], vertexList[k], vertexList[m]};
        res.push_back(tri);
    }
    return res;
}

Fragment constructFragment(int x, int y, float z, Triangle& tri, Vector3D& barycentric)
{
    Fragment frag;
    frag.screenPos.x = x;
    frag.screenPos.y = y;
    frag.zValue = z;
    for (int i = 0; i < 3; i++) frag.worldPos += tri[i].worldPos * barycentric[i];
    for (int i = 0; i < 3; i++) frag.normal += tri[i].normal * barycentric[i];
    for (int i = 0; i < 3; i++) frag.texUv += tri[i].texUv * barycentric[i];
    return frag;
}

CoordI4D renderAPI::computeBoundingBox(Triangle & tri)
{
    int xMin = width - 1;
    int yMin = height - 1;
    int xMax = 0;
    int yMax = 0;
    for(int i = 0; i < 3; i++)
    {
        xMin = std::min(xMin, tri[i].screenPos.x);
        yMin = std::min(yMin, tri[i].screenPos.y);
        xMax = std::max(xMax, tri[i].screenPos.x);
        yMax = std::max(yMax, tri[i].screenPos.y);
    }
    return {
        xMin > 0 ? xMin : 0,
        yMin > 0 ? yMin : 0,
        xMax < width - 1 ? xMax : width - 1,
        yMax < height - 1 ? yMax : height - 1};
}

Vector3D renderAPI::computeBarycentric(Triangle& pts, CoordI2D P) {
    Vector3D u1(pts[2].screenPos.x - pts[0].screenPos.x, pts[1].screenPos.x - pts[0].screenPos.x, pts[0].screenPos.x - P[0]);
    Vector3D u2(pts[2].screenPos.y - pts[0].screenPos.y, pts[1].screenPos.y - pts[0].screenPos.y, pts[0].screenPos.y - P[1]);
    Vector3D u = glm::cross(u1, u2);
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.z) < 1) return Vector3D(-1, 1, 1);
    return Vector3D(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

// initiaize clip plane and screen border
renderAPI::renderAPI(int _w, int _h):shader(nullptr), width(_w), height(_h), frame(width, height)
{
    {// set view planes
        // near
        viewBox[0] = {0, 0, 1.f, 1.f};
        // far
        viewBox[1] = {0, 0, -1.f, 1.f};
        // left
        viewBox[2] = {1.f, 0, 0, 1.f};
        // right
        viewBox[3] = {-1.f, 0, 0, 1.f};
        // top
        viewBox[4] = {0, 1.f, 0, 1.f};
        // bottom
        viewBox[5] = {0, -1.f, 0, 1.f};
    }
    {// set screen border
        // left
        screenEdge[0] = {1.f, 0, 0};
        // right
        screenEdge[1] = {-1.f, 0, (float)width};
        // bottom
        screenEdge[2] = {0, 1.f, 0};
        // top
        screenEdge[3] = {0, -1.f, (float)height};
    }
}

// half-space triangle rasterization algorithm
void renderAPI::facesRender(Triangle &tri)
{
    CoordI4D boundingBox = computeBoundingBox(tri);
    int xMin = boundingBox[0];
    int yMin = boundingBox[1];
    int xMax = boundingBox[2];
    int yMax = boundingBox[3];
    Vector3D P;
    Fragment frag;
    for (P.x = xMin; P.x <= xMax; P.x++)
    {
        for (P.y = yMin; P.y <= yMax; P.y++)
        {
            Vector3D bc_screen = computeBarycentric(tri, CoordI2D(P.x, P.y));
            if (judgeInsideTriangle(bc_screen)) {
                P.z = 0;
                for (int i = 0; i < 3; i++) P.z += tri[i].zValue * bc_screen[i];
                if (frame.updateZbuffer(P.x, P.y, P.z))
                {
                    frag = constructFragment(P.x, P.y, P.z, tri, bc_screen);
                    shader->fragmentShader(frag);
                    frame.setPixel(P.x, P.y, frag.fragmentColor);
                }
            }
        }
    }
}

// Bresenham's line algorithm
void renderAPI::drawLine(Line& line)
{
    int x0 = glm::clamp(static_cast<int>(line[0].x), 0, width - 1);
    int x1 = glm::clamp(static_cast<int>(line[1].x), 0, width - 1);
    int y0 = glm::clamp(static_cast<int>(line[0].y), 0, height - 1);
    int y1 = glm::clamp(static_cast<int>(line[1].y), 0, height - 1);
    bool steep = false;
    if (abs(x0 - x1) < abs(y0 - y1))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int k = dy > 0 ? 1 : -1;
    if (dy < 0)dy = -dy;
    float e = -dx;
    int x = x0, y = y0;
    while (x != x1)
    {
        if (steep)frame.setPixel(y, x, lineColor);
        else frame.setPixel(x, y, lineColor);
        e += (2 * dy);
        if (e > 0)
        {
            y += k;
            e -= (2 * dx);
        }
        ++x;
    }
}

// Cohen-Sutherland algorithm in screen space
std::optional<Line> renderAPI::lineClip(Line& line)
{
    std::bitset<4> code[2] =
    {
        getClipCode(Coord3D(line[0], 1), screenEdge),
        getClipCode(Coord3D(line[1], 1), screenEdge)
    };
    if((code[0] | code[1]).none())return line;
    if((code[0] & code[1]).any())return std::nullopt;
    for(int i = 0; i < 4; i++)
    {
        if((code[0] ^ code[1])[i])
        {
            float da = calculateDistance(Coord3D(line[0], 1), screenEdge[i]);
            float db = calculateDistance(Coord3D(line[1], 1), screenEdge[i]);
            float alpha = da / (da - db);
            CoordI2D np = calculateInterpolation(line[0], line[1], alpha);
            if(da > 0)
            {
                line[1] = np;
                code[1] = getClipCode(Coord3D(np, 1), screenEdge);
            }
            else
            {
                line[0] = np;
                code[0] = getClipCode(Coord3D(np, 1), screenEdge);
            }
        }
    }
    return line;
}

// WireframedTriangle
void renderAPI::wireframeRedner(Triangle &tri)
{
    Line triLine[3] =
    {
        {tri[0].screenPos, tri[1].screenPos},
        {tri[1].screenPos, tri[2].screenPos},
        {tri[2].screenPos, tri[0].screenPos},
    };
    for(auto &line : triLine)
    {
        auto res = lineClip(line);
        if(res) drawLine(*res);
    }
}

// render vertex
void renderAPI::pointsRender(Triangle &tri)
{
    for(int i = 0; i < 3; i++)
    {
        if(tri[i].screenPos.x >= 0 && tri[i].screenPos.x <= width - 1 &&
                tri[i].screenPos.y >= 0 && tri[i].screenPos.y <= height - 1 &&
                tri[i].zValue <= 1.f)
        {
            frame.setPixel(tri[i].screenPos.x, tri[i].screenPos.y,
                                 pointColor);
        }
    }
}

// clip triangle, Cohen-Sutherland algorithm & Sutherland-Hodgman algorithm in homogeneous space
std::vector<Triangle> renderAPI::faceClip(Triangle &tri)
{
    std::bitset<6> code[3] =
    {
        getClipCode(tri[0].clipPos, viewBox),
        getClipCode(tri[1].clipPos, viewBox),
        getClipCode(tri[2].clipPos, viewBox)
    };
    if((code[0] | code[1] | code[2]).none())
        return {tri};
    if((code[0] & code[1] & code[2]).any())
        return {};
    if(((code[0] ^ code[1])[0]) || ((code[1] ^ code[2])[0]) || ((code[2] ^ code[0])[0])) // intersects near plane
    {
        std::vector<Vertex> res;
        for(int i = 0; i < 3; i++)
        {
            int k = (i + 1) % 3;
            if(!code[i][0] && !code[k][0])
            {
                res.push_back(tri[k]);
            }
            else if(!code[i][0] && code[k][0])
            {
                float da = calculateDistance(tri[i].clipPos, viewBox[0]);
                float db = calculateDistance(tri[k].clipPos, viewBox[0]);
                float alpha = da / (da - db);
                Vertex np = calculateInterpolation(tri[i], tri[k], alpha);
                res.push_back(np);
            }
            else if(code[i][0] && !code[k][0])
            {
                float da = calculateDistance(tri[i].clipPos, viewBox[0]);
                float db = calculateDistance(tri[k].clipPos, viewBox[0]);
                float alpha = da / (da - db);
                Vertex np = calculateInterpolation(tri[i], tri[k], alpha);
                res.push_back(np);
                res.push_back(tri[k]);
            }
        }
        return constructTriangle(res);
    }
    return std::vector<Triangle>{tri};
}

// process triangle, clip triangle and choose one mode {TRIANGLE,LINE,POINT} to render.
void renderAPI::rasterization(Triangle &tri)
{
    for (int i = 0; i < 3; i++)
    {
        shader->vertexShader(tri[i]);
    }
    std::vector<Triangle> completedTriangleList = faceClip(tri);
    for (auto &ctri : completedTriangleList)
    {
        perspectiveTrans(ctri);
        convertToScreen(ctri);
        if(renderMode == FACE)facesRender(ctri);
        else if(renderMode == EDGE) wireframeRedner(ctri);
        else if(renderMode == VERTEX) pointsRender(ctri);
    }
}

// main render function
void renderAPI::render()
{
    std::vector<Triangle> triangleList;
    for (int i = 0; i < indices.size(); i += 3)
    {
        assert(i + 1 < indices.size() && i + 2 < indices.size());
        triangleList.push_back({vertexList.at(indices.at(i)), vertexList.at(indices.at(i + 1)), vertexList.at(indices.at(i + 2))});
    }
    if(multiThread)
    {
        for (int i = 0; i < triangleList.size(); i++)
            rasterization(triangleList[i]);
    }
    else
    {
        for(int i = 0; i < triangleList.size(); i++)
            rasterization(triangleList[i]);
    }
}