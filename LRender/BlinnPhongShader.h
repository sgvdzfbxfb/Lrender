#pragma once

#include "shader.h"
#include "texture.h"

class BlinnPhongShader : public Shader
{
public:
    virtual void vertexShader(Vertex &vertex) override;
    virtual void fragmentShader(Fragment &fragment) override;
};
