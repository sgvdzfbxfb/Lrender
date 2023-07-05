#pragma once

#include "shader.h"
#include "texture.h"

class BlinnPhongShader : public Shader
{
public:
    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 normal_matrix = glm::mat4(1.0f);
    virtual void vertexShader(Vertex &vertex, bool ifAnimation) override;
    virtual void fragmentShader(Fragment& fragment) override;
    void get_model_matrix(Vertex vertex);
};
