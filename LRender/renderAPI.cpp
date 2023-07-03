#include "renderAPI.h"
#include <QDebug>
#include <QTime>

// helper functions
static inline bool isInTriangle(Vector3D bc_screen)
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
        if(calculateDistance(point, clip.at(i)) < 0) res.set(i, 1);
    return res;
}

static Coord3D interpolate(float alpha, float beta, float gamma, const Coord3D& vert1, const Coord3D& vert2, const Coord3D& vert3, float weight)
{
    return (alpha * vert1 + beta * vert2 + gamma * vert3) / weight;
}

static Coord2D interpolate(float alpha, float beta, float gamma, const Coord2D& vert1, const Coord2D& vert2, const Coord2D& vert3, float weight)
{
    auto u = (alpha * vert1[0] + beta * vert2[0] + gamma * vert3[0]);
    auto v = (alpha * vert1[1] + beta * vert2[1] + gamma * vert3[1]);

    u /= weight;
    v /= weight;

    return Coord2D(u, v);
}

void renderAPI::perspectiveTrans(Triangle &tri)
{
    for (int i = 0; i < 3; i++)
    {
        tri.at(i).clipPos.x /= tri.at(i).clipPos.w;
        tri.at(i).clipPos.y /= tri.at(i).clipPos.w;
        tri.at(i).clipPos.z /= tri.at(i).clipPos.w;
    }
}

void renderAPI::convertToScreen(Triangle &tri)
{
    for (int i = 0; i < 3; i++)
    {
        tri.at(i).screenPos.x = static_cast<int>(0.5f * width * (tri.at(i).clipPos.x + 1.0f) + 0.5f);
        tri.at(i).screenPos.y = static_cast<int>(0.5f * height * (tri.at(i).clipPos.y + 1.0f) + 0.5f);
        tri.at(i).zValue = tri.at(i).clipPos.z;
    }
}

std::vector<Triangle> constructTriangle(std::vector<Vertex> vertexList)
{
    std::vector<Triangle> res;
    for(int i = 0; i < vertexList.size() - 2; i++)
    {
        int k = (i + 1) % vertexList.size();
        int m = (i + 2) % vertexList.size();
        Triangle tri{vertexList.at(0), vertexList.at(k), vertexList.at(m)};
        res.push_back(tri);
    }
    return res;
}

Fragment interpolationFragment(int x, int y, float z, Triangle& tri, Vector3D& barycentric)
{
    Fragment frag;
    frag.screenPos.x = x;
    frag.screenPos.y = y;
    frag.zValue = z;

    Vector3D bc_corrected = { 0, 0, 0 };
    for (int i = 0; i < 3; i++) bc_corrected[i] = barycentric[i] / tri[i].clipPos.w;
    float Z_n = 1. / (bc_corrected[0] + bc_corrected[1] + bc_corrected[2]);
    for (int i = 0; i < 3; i++) bc_corrected[i] *= Z_n;

    for (int i = 0; i < 3; i++) frag.worldPos += tri[i].worldPos * bc_corrected[i];
    for (int i = 0; i < 3; i++) frag.normal += tri[i].normal * bc_corrected[i];
    for (int i = 0; i < 3; i++) frag.texUv += tri[i].texUv * bc_corrected[i];

    return frag;
}

CoordI4D renderAPI::computeBoundingBox(Triangle& tri)
{
    int xMin = width - 1;
    int yMin = height - 1;
    int xMax = 0;
    int yMax = 0;
    for(int i = 0; i < 3; i++)
    {
        xMin = std::min(xMin, tri.at(i).screenPos.x);
        yMin = std::min(yMin, tri.at(i).screenPos.y);
        xMax = std::max(xMax, tri.at(i).screenPos.x);
        yMax = std::max(yMax, tri.at(i).screenPos.y);
    }
    return {
        xMin > 0 ? xMin : 0,
        yMin > 0 ? yMin : 0,
        xMax < width - 1 ? xMax : width - 1,
        yMax < height - 1 ? yMax : height - 1};
}

Vector3D renderAPI::computeBarycentric(Triangle& pts, CoordI2D P) {
    Vector3D u1(pts.at(2).screenPos.x - pts.at(0).screenPos.x, pts.at(1).screenPos.x - pts.at(0).screenPos.x, pts.at(0).screenPos.x - P[0]);
    Vector3D u2(pts.at(2).screenPos.y - pts.at(0).screenPos.y, pts.at(1).screenPos.y - pts.at(0).screenPos.y, pts.at(0).screenPos.y - P[1]);
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
    {
        // near
        viewBox.at(0) = {0, 0, 1.f, 1.f};
        // far
        viewBox.at(1) = {0, 0, -1.f, 1.f};
        // left
        viewBox.at(2) = {1.f, 0, 0, 1.f};
        // right
        viewBox.at(3) = {-1.f, 0, 0, 1.f};
        // top
        viewBox.at(4) = {0, 1.f, 0, 1.f};
        // bottom
        viewBox.at(5) = {0, -1.f, 0, 1.f};
    }
    {
        // left
        screenEdge.at(0) = {1.f, 0, 0};
        // right
        screenEdge.at(1) = {-1.f, 0, (float)width};
        // bottom
        screenEdge.at(2) = {0, 1.f, 0};
        // top
        screenEdge.at(3) = {0, -1.f, (float)height};
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
            Vector3D baryPos = computeBarycentric(tri, CoordI2D(P.x, P.y));
            if (isInTriangle(baryPos)) {
                float Z = 1.0 / (baryPos[0] / tri.at(0).clipPos.w + baryPos[1] / tri.at(1).clipPos.w + baryPos[2] / tri.at(2).clipPos.w);
                P.z = baryPos[0] * tri.at(0).clipPos.z + baryPos[1] * tri.at(1).clipPos.z + baryPos[2] * tri.at(2).clipPos.z;
                P.z *= Z / 1000.0;
                if (frame.updateZbuffer(P.x, P.y, P.z))
                {
                    frag = interpolationFragment(P.x, P.y, P.z, tri, baryPos);
                    shader->fragmentShader(frag);
                    frame.setPixel(P.x, P.y, frag.fragmentColor);
                }
            }
        }
    }
}

void renderAPI::skyBoxFacesRender(Triangle& tri, int faceId)
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
            Vector3D bcPos = computeBarycentric(tri, CoordI2D(P.x, P.y));
            if (isInTriangle(bcPos)) {
                float Z = 1.0 / (bcPos[0] / tri.at(0).clipPos.w + bcPos[1] / tri.at(1).clipPos.w + bcPos[2] / tri.at(2).clipPos.w);
                P.z = bcPos[0] * tri.at(0).clipPos.z / tri.at(0).clipPos.w + bcPos[1] * tri.at(1).clipPos.z / tri.at(1).clipPos.w + bcPos[2] * tri.at(2).clipPos.z / tri.at(2).clipPos.w;
                P.z *= Z;
                if (frame.updateZbuffer(P.x, P.y, P.z))
                {
                    frag = interpolationFragment(P.x, P.y, P.z, tri, bcPos);
                    skyShader->fragmentShader(frag, faceId);
                    frame.setPixel(P.x, P.y, frag.fragmentColor);
                }
            }
        }
    }
}

// Bresenham's line algorithm
void renderAPI::drawLine(Line& line)
{
    int x0 = glm::clamp(static_cast<int>(line.at(0).x), 0, width - 1);
    int x1 = glm::clamp(static_cast<int>(line.at(1).x), 0, width - 1);
    int y0 = glm::clamp(static_cast<int>(line.at(0).y), 0, height - 1);
    int y1 = glm::clamp(static_cast<int>(line.at(1).y), 0, height - 1);
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
        getClipCode(Coord3D(line.at(0), 1), screenEdge),
        getClipCode(Coord3D(line.at(1), 1), screenEdge)
    };
    if((code[0] | code[1]).none())return line;
    if((code[0] & code[1]).any())return std::nullopt;
    for(int i = 0; i < 4; i++)
    {
        if((code[0] ^ code[1])[i])
        {
            float da = calculateDistance(Coord3D(line.at(0), 1), screenEdge.at(i));
            float db = calculateDistance(Coord3D(line.at(1), 1), screenEdge.at(i));
            float alpha = da / (da - db);            
            CoordI2D np = calculateInterpolation(line.at(0), line.at(1), alpha);
            if(da > 0)
            {
                line.at(1) = np;
                code[1] = getClipCode(Coord3D(np, 1), screenEdge);
            }
            else
            {
                line.at(0) = np;
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
        {tri.at(0).screenPos, tri.at(1).screenPos},
        {tri.at(1).screenPos, tri.at(2).screenPos},
        {tri.at(2).screenPos, tri.at(0).screenPos},
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
        if(tri.at(i).screenPos.x >= 0 && tri.at(i).screenPos.x <= width - 1 &&
                tri.at(i).screenPos.y >= 0 && tri.at(i).screenPos.y <= height - 1 &&
                tri.at(i).zValue <= 1.f)
        {
            frame.setPixel(tri.at(i).screenPos.x, tri.at(i).screenPos.y,
                                 pointColor);
        }
    }
}

// clip triangle, Cohen-Sutherland algorithm & Sutherland-Hodgman algorithm in homogeneous space
std::vector<Triangle> renderAPI::faceClip(Triangle &tri)
{
    std::bitset<6> code[3] =
    {
        getClipCode(tri.at(0).clipPos, viewBox),
        getClipCode(tri.at(1).clipPos, viewBox),
        getClipCode(tri.at(2).clipPos, viewBox)
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
                res.push_back(tri.at(k));
            }
            else if(!code[i][0] && code[k][0])
            {
                float da = calculateDistance(tri.at(i).clipPos, viewBox.at(0));
                float db = calculateDistance(tri.at(k).clipPos, viewBox.at(0));
                float alpha = da / (da - db);
                Vertex np = calculateInterpolation(tri.at(i), tri.at(k), alpha);
                res.push_back(np);
            }
            else if(code[i][0] && !code[k][0])
            {
                float da = calculateDistance(tri.at(i).clipPos, viewBox.at(0));
                float db = calculateDistance(tri.at(k).clipPos, viewBox.at(0));
                float alpha = da / (da - db);
                Vertex np = calculateInterpolation(tri.at(i), tri.at(k), alpha);
                res.push_back(np);
                res.push_back(tri.at(k));
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
        shader->vertexShader(tri.at(i));
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
    if(multiThread)
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, faces.size(), 2500),
            [&](tbb::blocked_range<size_t> r)
            {
                for (size_t i = r.begin(); i < r.end(); i++)
                    rasterization(faces.at(i));
            });
    }
    else
    {
        for(int i = 0; i < faces.size(); i++)
            rasterization(faces.at(i));
    }
}

// render skybox
void renderAPI::renderSkyBox()
{
    if (multiThread)
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, SkyBoxFaces.size()),
            [&](tbb::blocked_range<size_t> r)
            {
                for (size_t i = r.begin(); i < r.end(); i++) {
                    for (int j = 0; j < 3; j++)
                    {
                        skyShader->vertexShader(SkyBoxFaces.at(i).at(j));
                    }
                    std::vector<Triangle> completedTriangleList = faceClip(SkyBoxFaces.at(i));
                    for (int j = 0; j < completedTriangleList.size(); j++)
                    {
                        perspectiveTrans(completedTriangleList[j]);
                        convertToScreen(completedTriangleList[j]);
                        skyBoxFacesRender(completedTriangleList[j], i);
                    }
                }
            });
    }
    else
    {
        for (int i = 0; i < SkyBoxFaces.size(); i++) {
            for (int j = 0; j < 3; j++)
            {
                skyShader->vertexShader(SkyBoxFaces.at(i).at(j));
            }
            std::vector<Triangle> completedTriangleList = faceClip(SkyBoxFaces.at(i));
            for (int j = 0; j < completedTriangleList.size(); j++)
            {
                perspectiveTrans(completedTriangleList[j]);
                convertToScreen(completedTriangleList[j]);
                skyBoxFacesRender(completedTriangleList[j], i);
            }
        }
    }
}
