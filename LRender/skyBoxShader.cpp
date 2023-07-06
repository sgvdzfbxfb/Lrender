#include "skyBoxShader.h"

std::vector<Coord2D> blurSkyBox(Coord2D uv)
{
    std::vector<Coord2D> aveUv;
    aveUv.push_back(Coord2D(uv.x, uv.y));
    aveUv.push_back(Coord2D(uv.x, uv.y + 1.0));
    aveUv.push_back(Coord2D(uv.x, uv.y - 1.0));
    aveUv.push_back(Coord2D(uv.x + 1.0, uv.y));
    aveUv.push_back(Coord2D(uv.x + 1.0, uv.y + 1.0));
    aveUv.push_back(Coord2D(uv.x + 1.0, uv.y - 1.0));
    aveUv.push_back(Coord2D(uv.x - 1.0, uv.y));
    aveUv.push_back(Coord2D(uv.x - 1.0, uv.y + 1.0));
    aveUv.push_back(Coord2D(uv.x - 1.0, uv.y - 1.0));
    return aveUv;
}

void SkyBoxShader::vertexShader(Vertex& vertex)
{
    vertex.worldPos = Coord3D(modelMat * Coord4D(vertex.worldPos, 1.f));
    vertex.clipPos = projectionMat * viewMat * Coord4D(vertex.worldPos, 1.f);
}

void SkyBoxShader::fragmentShader(Fragment& fragment, int faceId)
{
    Vector3D viewDir = glm::normalize(eyePos - Coord3D(fragment.screenPos, 1.0));
    Color result = renderAPI::API().skyBoxTexture[faceId / 2.0].getColorFromUv(fragment.texUv);
    fragment.fragmentColor = result;
}
