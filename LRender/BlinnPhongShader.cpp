#include "blinnPhongShader.h"

glm::mat4 mat4_combine(glm::mat4 m[4], Vector4D weights_) {
    glm::mat4 combined(0.0);
    float weights[4];
    int i, r, c;

    weights[0] = weights_.x;
    weights[1] = weights_.y;
    weights[2] = weights_.z;
    weights[3] = weights_.w;

    for (i = 0; i < 4; i++) {
        float weight = weights[i];
        if (weight > 0) {
            glm::mat4 source = m[i];
            for (r = 0; r < 4; r++) {
                for (c = 0; c < 4; c++) {
                    combined[r][c] += weight * source[r][c];
                }
            }
        }
    }

    return combined;
}

void BlinnPhongShader::get_model_matrix(Vertex vertex) {
    if (joint_matrices.size()) {
        glm::mat4 js[4];
        js[0] = joint_matrices[vertex.joint.x];
        js[1] = joint_matrices[vertex.joint.y];
        js[2] = joint_matrices[vertex.joint.z];
        js[3] = joint_matrices[vertex.joint.w];
        model_matrix = mat4_combine(js, vertex.weight);

        glm::mat4 joint_ns[4];
        joint_ns[0] = joint_n_matrices[vertex.joint.x];
        joint_ns[1] = joint_n_matrices[vertex.joint.y];
        joint_ns[2] = joint_n_matrices[vertex.joint.z];
        joint_ns[3] = joint_n_matrices[vertex.joint.w];
        normal_matrix = mat4_combine(joint_ns, vertex.weight);
    }
}

void BlinnPhongShader::vertexShader(Vertex &vertex, bool ifAnimation)
{
    if (ifAnimation) get_model_matrix(vertex);
    vertex.worldPos = Coord3D(model_matrix * Coord4D(vertex.worldPos, 1.f));
    vertex.normal = Coord3D(normal_matrix * Coord4D(vertex.normal, 1.f));

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
