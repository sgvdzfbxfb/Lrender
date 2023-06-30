#include "blinnPhongShader.h"

void BlinnPhongShader::vertexShader(Vertex &vertex)
{
    vertex.worldPos = Coord3D(modelMat * Coord4D(vertex.worldPos, 1.f));
    vertex.clipPos = projectionMat * viewMat * Coord4D(vertex.worldPos, 1.f);
    vertex.normal = glm::mat3(glm::transpose(glm::inverse(modelMat))) * vertex.normal;
}

void BlinnPhongShader::fragmentShader(Fragment &fragment)
{
    Color diffuseColor = {0.0f,0.0f,0.0f};
    Color specularColor = {0.0f,0.0f,0.0f};
    if (material.diffuse.size() != 0) {
        for (int i = 0; i < material.diffuse.size(); ++i)
            diffuseColor += renderAPI::API().textureList[material.diffuse.at(i)].getColorFromUv(fragment.texUv);
        diffuseColor /= material.diffuse.size();
    }
    else diffuseColor = { 0.6f,0.6f,0.6f };
    if (material.specular.size() != 0) {
        for (int i = 0; i < material.specular.size(); ++i)
            specularColor = renderAPI::API().textureList[material.specular.at(i)].getColorFromUv(fragment.texUv);
        specularColor /= material.specular.size();
    }
    else specularColor = { 1.0f,1.0f,1.0f };
    Vector3D normal = glm::normalize(fragment.normal);
    Vector3D viewDir = glm::normalize(eyePos - fragment.worldPos);
    auto calculateLight = [&](Light light)->Color
    {
        Vector3D lightDir;
        if(light.pos.w != 0.f)
            lightDir = glm::normalize(Coord3D(light.pos) - fragment.worldPos);
        else
            lightDir = -Vector3D(light.dir);
        Color ambient = light.ambient * diffuseColor;
        Color diffuse = light.diffuse * std::max(glm::dot(normal,lightDir), 0.f) * diffuseColor;
        Color specular = light.specular * std::pow(std::max(glm::dot(normal, glm::normalize(viewDir + lightDir)), 0.0f), material.shininess) * specularColor;
        return (ambient + diffuse + specular);
    };
    Color result(0.f, 0.f, 0.f);
    for(int i = 0; i < lightList.size(); ++i)
    {
        result += calculateLight(lightList.at(i));
    }
    if(result.x > 1.f) result.x = 1.f;
    if(result.y > 1.f) result.y = 1.f;
    if(result.z > 1.f) result.z = 1.f;
    fragment.fragmentColor = result;
}
