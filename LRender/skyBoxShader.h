#pragma once

#include "shader.h"
#include "texture.h"

class SkyBoxShader
{
public:
    glm::mat4 modelMat;
    glm::mat4 viewMat;
    glm::mat4 projectionMat;
    std::vector<Light> lightList;
    Material material;
    Coord3D eyePos;
    void vertexShader(Vertex &vertex);
    void fragmentShader(Fragment& fragment, int faceId);
};
