#include "skyBoxShader.h"


void SkyBoxShader::vertexShader(Vertex& vertex)
{
    vertex.worldPos = Coord3D(modelMat * Coord4D(vertex.worldPos, 1.f));
    vertex.clipPos = projectionMat * viewMat * Coord4D(vertex.worldPos, 1.f);
}

void SkyBoxShader::fragmentShader(Fragment& fragment, int faceId)
{
    Vector3D viewDir = glm::normalize(eyePos - Coord3D(fragment.screenPos, 1.0));
    Color result = renderAPI::API().skyBox[faceId / 2.0].getColorFromUv(fragment.texUv);
    fragment.fragmentColor = result;
}
