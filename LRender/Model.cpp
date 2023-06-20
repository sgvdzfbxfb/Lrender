#include "Model.h"
#include <QDebug>

Model::Model(QString path)
{
    loadModel(path);
    centre = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};
}

void Model::Draw()
{
    SRendererDevice::GetInstance().textureList = textureList;
    for(int i = 0; i < meshes.size(); i++)
        meshes[i].Draw();
}

void Model::loadModel(QString path)
{
    Mesh tempMesh;
    std::ifstream in;
    in.open(path.toStdString(), std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vertex ver;
            Vector3D v_p;
            for (int i = 0; i < 3; i++) iss >> v_p[i];

            minX = std::min(minX, v_p.x);
            minY = std::min(minY, v_p.y);
            minZ = std::min(minZ, v_p.z);
            maxX = std::max(maxX, v_p.x);
            maxY = std::max(maxY, v_p.y);
            maxZ = std::max(maxZ, v_p.z);
            ver.worldSpacePos = v_p;
            /*if (mesh->mTextureCoords[0])
                ver.texCoord = Coord2D(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                ver.texCoord = Coord2D(0.0f, 0.0f);*/
            tempMesh.vertices.push_back(ver);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<unsigned> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                tempMesh.indices.push_back(idx);
                f.push_back(idx);
                (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
            }
            tempMesh.faces.push_back(f);
        }
    }
    vertexCount += tempMesh.vertices.size();
    triangleCount += tempMesh.faces.size();
    meshes.push_back(tempMesh);
}

void Model::computeNormal()
{
    for (int l = 0; l < meshes.size(); ++l) {
        std::vector<Vector3D> faceNormals;
        for (int i = 0; i < meshes[l].indices.size(); i += 3) {
            Vector3D AB = (meshes[l].vertices[(meshes[l].indices[i + 1])]).worldSpacePos - (meshes[l].vertices[(meshes[l].indices[i])]).worldSpacePos;
            Vector3D AC = (meshes[l].vertices[(meshes[l].indices[i + 2])]).worldSpacePos - (meshes[l].vertices[(meshes[l].indices[i])]).worldSpacePos;
            Vector3D faceN = glm::normalize(glm::cross(AB, AC));
            faceNormals.push_back(faceN);
        }
        for (int i = 0; i < meshes[l].vertices.size(); ++i) {
            std::vector<int> tempMs = meshes[l].verToFace[i];
            Vector3D verNave(0.0, 0.0, 0.0);
            for (int j = 0; j < tempMs.size(); ++j) {
                verNave += faceNormals[tempMs[j]];
            }
            verNave /= tempMs.size();
            verNave = glm::normalize(verNave);
            meshes[l].vertices[i].normal = verNave;
        }
    }
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    vertexCount += mesh->mNumVertices;
    triangleCount += mesh->mNumFaces;
    Mesh res;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Vertex vertex;
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        minX = std::min(minX, mesh->mVertices[i].x);
        minY = std::min(minY, mesh->mVertices[i].y);
        minZ = std::min(minZ, mesh->mVertices[i].z);
        maxX = std::max(maxX, mesh->mVertices[i].x);
        maxY = std::max(maxY, mesh->mVertices[i].y);
        maxZ = std::max(maxZ, mesh->mVertices[i].z);
        vertex.worldSpacePos = Coord3D(mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z);
        vertex.normal = Coord3D(mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z);
        if(mesh->mTextureCoords[0])//only process one texture
            vertex.texCoord = Coord2D(mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y);
        else
            vertex.texCoord = Coord2D(0.0f,0.0f);
        vertices.push_back(vertex);
    }
    res.vertices = vertices;
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    res.indices = indices;
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        res.diffuseTextureIndex = loadMaterialTextures(res, material, aiTextureType_DIFFUSE);
        res.specularTextureIndex = loadMaterialTextures(res, material, aiTextureType_SPECULAR);
    }
    return res;
}

int Model::loadMaterialTextures(Mesh &mesh, aiMaterial *mat, aiTextureType type)
{
    if(mat->GetTextureCount(type) > 0)
    {
        aiString str;
        mat->GetTexture(type, 0, &str);
        QString path = directory + '/' + str.C_Str();
        for(int i = 0; i < textureList.size(); i++)
        {
            if(textureList[i].path == path)
                return i;
        }
        Texture texture;
        if(texture.LoadFromImage(path))
        {
            qDebug() << path;
            textureList.push_back(texture);
            return (int)textureList.size() - 1;
        }
    }
    return -1;
}


