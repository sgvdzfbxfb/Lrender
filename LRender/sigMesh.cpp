#include "sigMesh.h"
#include <QDebug>
void sigMesh::meshRender()
{
    renderAPI::API().vertexList = vertices;
    renderAPI::API().indices = indices;
    renderAPI::API().shader->material.diffuse = diffuseIds;
    renderAPI::API().shader->material.specular = specularIds;
    renderAPI::API().render();
}
