#include "sigMesh.h"
#include <QDebug>
void sigMesh::draw()
{
    renderAPI::API().vertexList = vertices;
    renderAPI::API().indices = indices;
    renderAPI::API().shader->material.diffuse = diffuseTextureIndex;
    renderAPI::API().shader->material.specular = specularTextureIndex;
    renderAPI::API().render();
}
