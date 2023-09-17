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
