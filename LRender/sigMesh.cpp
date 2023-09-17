#include "sigMesh.h"
#include <QDebug>
void sigMesh::meshRender()
{
    renderAPI::API().faces = faces;
    renderAPI::API().shader->material.diffuse = diffuseIds;
    renderAPI::API().shader->material.specular = specularIds;
    renderAPI::API().render(ifAnimation);
    app_ani_faces = renderAPI::API().faces;
}
