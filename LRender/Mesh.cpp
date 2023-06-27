#include "Mesh.h"
#include <QDebug>
void Mesh::Draw()
{
    renderAPI::GetInstance().vertexList = vertices;
    renderAPI::GetInstance().indices = indices;
    renderAPI::GetInstance().shader->material.diffuse = diffuseTextureIndex;
    renderAPI::GetInstance().shader->material.specular = specularTextureIndex;
    renderAPI::GetInstance().Render();
}
