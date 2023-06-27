#include "blinnPhongShader.h"

void BlinnPhongShader::vertexShader(Vertex &vertex)
{
    vertex.worldSpacePos = Coord3D(modelMat * Coord4D(vertex.worldSpacePos, 1.f));
    vertex.clipSpacePos = projectionMat * viewMat * Coord4D(vertex.worldSpacePos, 1.f);
    vertex.normal = glm::mat3(glm::transpose(glm::inverse(modelMat))) * vertex.normal;
}

void BlinnPhongShader::fragmentShader(Fragment &fragment)
{
    Color diffuseColor = {0.6f,0.6f,0.6f};
    Color specularColor = {1.0f,1.0f,1.0f};
    if(material.diffuse != -1) diffuseColor = renderAPI::API().textureList[material.diffuse].sample2D(fragment.texCoord);
    if(material.specular != -1) specularColor = renderAPI::API().textureList[material.specular].sample2D(fragment.texCoord);
    Vector3D normal = glm::normalize(fragment.normal);
    Vector3D viewDir = glm::normalize(eyePos - fragment.worldSpacePos);
    auto calculateLight = [&](Light light)->Color
    {
        Vector3D lightDir;
        if(light.pos.w != 0.f)
            lightDir = glm::normalize(Coord3D(light.pos) - fragment.worldSpacePos);
        else
            lightDir = -Vector3D(light.dir);
        Color ambient = light.ambient * diffuseColor;
        Color diffuse = light.diffuse * std::max(glm::dot(normal,lightDir), 0.f) * diffuseColor;
        Color specular = light.specular * std::pow(std::max(glm::dot(normal, glm::normalize(viewDir + lightDir)), 0.0f), material.shininess) * specularColor;
        return (ambient + diffuse + specular);
    };
    Color result(0.f, 0.f, 0.f);
    for(auto light : lightList)
    {
        result += calculateLight(light);
    }
    if(result.x > 1.f) result.x = 1.f;
    if(result.y > 1.f) result.y = 1.f;
    if(result.z > 1.f) result.z = 1.f;
    fragment.fragmentColor = result;
}
