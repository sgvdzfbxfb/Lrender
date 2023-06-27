#pragma once
#pragma warning(disable : 4251)

#include <cmath>
#include <string>
#include <functional>
#include "renderAPI.h"
#include "dataType.h"

class renderAPI;

class Shader
{
public:
    glm::mat4 modelMat;
    glm::mat4 viewMat;
    glm::mat4 projectionMat;
    std::vector<Light> lightList;
    Material material;
    Coord3D eyePos;
    virtual void vertexShader(Vertex &vertex) = 0;
    virtual void fragmentShader(Fragment &fragment) = 0;
};
