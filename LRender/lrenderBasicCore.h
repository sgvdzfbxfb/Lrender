#pragma once
#pragma warning(disable : 4251)

#include <QImage>
#include <QColor>
#include <QString>
#include <string>
#include <any>
#include <cassert>
#include <cmath>
#include <random>
#include <iostream>
#include <array>
#include "glm/glm.hpp"

const float EPSILON = 0.00001;
const float kInfinity = std::numeric_limits<float>::max();

using Color = glm::vec3;
using Vector2D = glm::vec2;
using Vector3D = glm::vec3;
using VectorI3D = glm::ivec3;
using Vector4D = glm::vec4;
using VectorI4D = glm::ivec4;
using Coord2D = glm::vec2;
using CoordI2D = glm::ivec2;
using Coord3D = glm::vec3;
using CoordI3D = glm::ivec3;
using Coord4D = glm::vec4;
using CoordI4D = glm::ivec4;
using BorderPlane = glm::vec4;
using BorderLine = glm::vec3;

enum renderMode{FACE,EDGE,VERTEX};
enum renderFigure{BACKGROUND, LINE, POINT};
enum lightColorType{DIFFUSE, SPECULAR, AMBIENT};

struct Vertex
{
    Coord3D worldPos = Coord3D(0.0, 0.0, 0.0);
    Coord4D clipPos = Coord4D(0.0, 0.0, 0.0, 0.0);
    CoordI2D screenPos = CoordI2D(0, 0);
    float zValue = FLT_MAX;
    Vector3D normal = Vector3D(0.0, 0.0, 0.0);
    Coord2D texUv = Coord2D(0.0, 0.0);
    VectorI4D joint = VectorI4D(0, 0, 0, 0);
    Vector4D weight = Vector4D(0.0, 0.0, 0.0, 0.0);
};

class BVHItem
{
public:
    BVHItem() {}
    virtual ~BVHItem() {}
    virtual bool intersect(const Ray& ray) = 0;
    virtual bool intersect(const Ray& ray, float&, uint32_t&) const = 0;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual void getSurfaceProperties(const Vector3D&, const Vector3D&, const uint32_t&, const Vector2D&, Vector3D&, Vector2D&) const = 0;
    virtual Vector3D evalDiffuseColor(const Vector2D&) const = 0;
    virtual Bounds3 getBounds() = 0;
    virtual float getArea() = 0;
    virtual void Sample(Intersection& pos, float& pdf) = 0;
    virtual bool hasEmit() = 0;
};

class Triangle : public BVHItem
{
public:
    Vertex v0, v1, v2; // vertices A, B ,C , counter-clockwise order
    Vector3D e1, e2;     // 2 edges v1-v0, v2-v0;
    Vector3D normal;
    float area;
    Material* m;

    Triangle(Vertex _v0, Vertex _v1, Vertex _v2, Material* _m = nullptr);
    bool intersect(const Ray& ray) override;
    bool intersect(const Ray& ray, float& tnear, uint32_t& index) const override;
    Intersection getIntersection(Ray ray) override;
    void getSurfaceProperties(const Vector3D& P, const Vector3D& I, const uint32_t& index, const Vector2D& uv, Vector3D& N, Vector2D& st) const override;
    Vector3D evalDiffuseColor(const Vector2D&) const override;
    Bounds3 getBounds() override;
    void Sample(Intersection& pos, float& pdf);
    float getArea();
    bool hasEmit();
};

using Line = std::array<CoordI2D, 2>;

struct Fragment
{
    Coord3D worldPos = Coord3D(0.0, 0.0, 0.0);
    CoordI2D screenPos = CoordI2D(0, 0);
    float zValue = FLT_MAX;
    Color fragmentColor = Color(0.0, 0.0, 0.0);
    Vector3D normal = Vector3D(0.0, 0.0, 0.0);
    Coord2D texUv = Coord2D(0.0, 0.0);
};

struct Light
{
    union{
    Coord4D pos;
    Vector4D dir;
    };
    Color ambient;
    Color diffuse;
    Color specular;
};

struct Material
{
    std::vector<int> diffuse;
    std::vector<int> specular;
    float shininess;
};

struct Ray {
    //Destination = origin + t*direction
    Coord3D origin;
    Vector3D direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max;

    Ray(const Coord3D& ori, const Vector3D& dir, const double _t = 0.0) : origin(ori), direction(dir), t(_t) {
        direction_inv = Vector3D(1. / direction.x, 1. / direction.y, 1. / direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();
    }

    Vector3D operator()(double t) const { return origin + Vector3D(direction.x * t, direction.y * t, direction.z * t); }
};

struct Intersection
{
    Intersection() {
        happened = false;
        coords = Vector3D();
        normal = Vector3D();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }
    bool happened;
    Vector3D coords;
    Vector3D tcoords;
    Vector3D normal;
    double distance;
    Triangle* obj;
    Material* m;
};

class Bounds3
{
public:
    Vector3D pMin, pMax; // two points to specify the bounding box
    Bounds3();
    Bounds3(const Vector3D p);
    Bounds3(const Vector3D p1, const Vector3D p2);

    Vector3D Diagonal() const;
    int maxExtent() const;

    double SurfaceArea() const;

    Vector3D Centroid();
    Bounds3 Intersect(const Bounds3& b);

    Vector3D Offset(const Vector3D& p) const;

    bool Overlaps(const Bounds3& b1, const Bounds3& b2);

    bool Inside(const Vector3D& p, const Bounds3& b);
    inline const Vector3D& operator[](int i) const;

    inline bool IntersectP(const Ray& ray, const Vector3D& invDir, const std::array<int, 3>& dirisNeg) const;
};


inline float clamp(const float& lo, const float& hi, const float& v)
{
    return std::max(lo, std::min(hi, v));
}

inline  bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = -0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}

inline float get_random_float()
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

    return dist(rng);
}

inline void UpdateProgress(float progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};
