#pragma once

#include "Shader.hpp"
#include "Texture.h"

class BlinnPhongShader : public Shader
{
public:
    virtual void VertexShader(Vertex &vertex) override;
    virtual void FragmentShader(Fragment &fragment) override;
};
