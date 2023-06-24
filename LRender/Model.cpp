#include "Model.h"
#include <QDebug>

Model::Model(QStringList paths)
{
    loadModel(paths);
    centre = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};
}

void Model::Draw()
{
    SRendererDevice::GetInstance().textureList = textureList;
    for(int i = 0; i < meshes.size(); i++)
        meshes[i].Draw();
}

void Model::loadModel(QStringList paths)
{
    QList<QString>::Iterator path = paths.begin(), itend = paths.end();
    int i = 0;
    for (; path != itend; path++, i++) {
        Mesh tempMesh;
        std::ifstream in;
        in.open(path->toStdString(), std::ifstream::in);
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
                tempMesh.vertices.push_back(ver);
            }
            else if (!line.compare(0, 3, "vn ")) {
                iss >> trash >> trash;
                Vector3D n;
                for (int i = 0; i < 3; i++) iss >> n[i];
                tempMesh.vertNormals.push_back(n);
            }
            else if (!line.compare(0, 3, "vt ")) {
                iss >> trash >> trash;
                Coord2D uv;
                for (int i = 0; i < 2; i++) iss >> uv[i];
                tempMesh.vertUVs.push_back(uv);
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
        tempMesh.diffuseTextureIndex = loadMaterialTextures(*(path), "diffuse");
        tempMesh.specularTextureIndex = loadMaterialTextures(*(path), "specular");
        meshes.push_back(tempMesh);
    }
    
    for (int l = 0; l < meshes.size(); ++l) {
        if (meshes[l].vertNormals.size() == meshes[l].vertices.size()) {
            for (int i = 0; i < meshes[l].vertices.size(); ++i) {
                meshes[l].vertices[i].normal = meshes[l].vertNormals[i];
            }
        }
        else computeNormal(l);

        if (meshes[l].vertUVs.size() == meshes[l].vertices.size()) {
            for (int i = 0; i < meshes[l].vertices.size(); ++i) {
                meshes[l].vertices[i].texCoord = meshes[l].vertUVs[i];
            }
        }
        else {
            for (int i = 0; i < meshes[l].vertices.size(); ++i) {
                meshes[l].vertices[i].texCoord = Coord2D(0.0f, 0.0f);
            }
        }
    }
}

void Model::computeNormal(int meshIdx)
{
    std::vector<Vector3D> faceNormals;
    for (int i = 0; i < meshes[meshIdx].indices.size(); i += 3) {
        Vector3D AB = (meshes[meshIdx].vertices[(meshes[meshIdx].indices[i + 1])]).worldSpacePos - (meshes[meshIdx].vertices[(meshes[meshIdx].indices[i])]).worldSpacePos;
        Vector3D AC = (meshes[meshIdx].vertices[(meshes[meshIdx].indices[i + 2])]).worldSpacePos - (meshes[meshIdx].vertices[(meshes[meshIdx].indices[i])]).worldSpacePos;
        Vector3D faceN = glm::normalize(glm::cross(AB, AC));
        faceNormals.push_back(faceN);
    }
    for (int i = 0; i < meshes[meshIdx].vertices.size(); ++i) {
        std::vector<int> tempMs = meshes[meshIdx].verToFace[i];
        Vector3D verNave(0.0, 0.0, 0.0);
        for (int j = 0; j < tempMs.size(); ++j) {
            verNave += faceNormals[tempMs[j]];
        }
        verNave /= tempMs.size();
        verNave = glm::normalize(verNave);
        meshes[meshIdx].vertices[i].normal = verNave;
    }
}

int Model::loadMaterialTextures(QString path, std::string type)
{
    QString t_p = path;
    std::ifstream diffuse;
    t_p.chop(4);
    std::string t_ps = t_p.toStdString() + "_" + type + ".png";
    diffuse.open(t_ps, std::ifstream::in);
    if (diffuse.fail()) return -1;

    Texture texture;
    if (texture.LoadFromImage(QString::fromStdString(t_ps)))
    {
        qDebug() << QString::fromStdString(t_ps);
        textureList.push_back(texture);
        return (int)textureList.size() - 1;
    }
    else return -1;
}


