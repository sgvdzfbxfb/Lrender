#pragma once
#pragma warning(disable : 4251)

#include <QImage>
#include <QColor>
#include <QString>
#include <string>
#include <any>
#include <cassert>
#include <cmath>
#include <iostream>
#include <array>
#include "glm/glm.hpp"

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

using Triangle = std::array<Vertex, 3>;
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
