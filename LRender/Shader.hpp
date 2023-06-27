#pragma once
#pragma warning(disable : 4251)

#include <cmath>
#include <string>
#include <functional>
#include "renderAPI.h"
#include "BasicDataStructure.hpp"

class renderAPI;

class Shader
{
public:
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Projection;
    std::vector<Light> lightList;
    Material material;
    Coord3D eyePos;
    virtual void VertexShader(Vertex &vertex) = 0;
    virtual void FragmentShader(Fragment &fragment) = 0;
};
