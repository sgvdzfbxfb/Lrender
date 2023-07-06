#pragma once

#include "shader.h"
#include "texture.h"

class BlinnPhongShader : public Shader
{
public:
    virtual void vertexShader(Vertex &vertex, bool ifAnimation) override;
    virtual void fragmentShader(Fragment& fragment) override;
    glm::mat4 get_model_matrix(Vertex vertex);
};
