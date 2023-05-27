#pragma once

#include "src/Shader.hpp"
#include "src/Texture.h"
#include "src/SRendererCoreExport.h"

class BlinnPhongShader : public Shader
{
public:
    virtual void VertexShader(Vertex &vertex) override;
    virtual void FragmentShader(Fragment &fragment) override;
};
