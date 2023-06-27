#include "model.h"
#include <QDebug>

std::vector<std::string> splitOBJpath(const std::string& str, const std::string& delim) {
    std::vector<std::string> res;
    if ("" == str) return res;
    char* strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char* d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char* p = strtok(strs, d);
    while (p) {
        std::string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }
    return res;
}

Model::Model(QStringList paths)
{
    loadModel(paths);
    modelCenter = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};
}

void Model::letModelRender()
{
    renderAPI::API().textureList = textureList;
    for(int i = 0; i < meshes.size(); i++)
        meshes[i].letMeshRender();
}

void Model::loadModel(QStringList paths)
{
    QList<QString>::Iterator path = paths.begin(), itend = paths.end();
    int i = 0;
    for (; path != itend; path++, i++) {
        sigMesh tempMesh;
        std::ifstream in, in_forCount;
        in.open(path->toStdString(), std::ifstream::in);
        in_forCount.open(path->toStdString(), std::ifstream::in);
        if (in.fail() || in_forCount.fail()) return;
        std::string line;
        int v_count = 0, vn_count = 0, vt_count = 0, f_count = 0;
        while (!in_forCount.eof()) {
            std::getline(in_forCount, line);
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
                ver.worldPos = v_p;
                tempMesh.vertices.push_back(ver);
                v_count++;
            }
            else if (!line.compare(0, 3, "vn ")) {
                iss >> trash >> trash;
                Vector3D n;
                for (int i = 0; i < 3; i++) iss >> n[i];
                tempMesh.vertNormals.push_back(n);
                vn_count++;
            }
            else if (!line.compare(0, 3, "vt ")) {
                iss >> trash >> trash;
                Coord2D uv;
                for (int i = 0; i < 2; i++) iss >> uv[i];
                tempMesh.vertUVs.push_back(uv);
                vt_count++;
            }
            else if (!line.compare(0, 2, "f ")) {
                f_count++;
            }
        }
        while (!in.eof()) {
            std::getline(in, line);
            if (!line.compare(0, 2, "f ")) {
                std::vector<unsigned> f;
                int idx, vn_idx, vt_idx;
                std::vector<std::string> frg_res = splitOBJpath(line, " ");
                //qDebug() << "frg_res" << frg_res.size();
                if (vn_count == 0 && vt_count == 0) {
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitOBJpath(frg_res[k], "/");
                        idx = atoi(idxs[0].c_str());
                        idx--;
                        tempMesh.indices.push_back(idx);
                        f.push_back(idx);
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                    }
                }
                else if (vn_count != 0 && vt_count == 0) {
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitOBJpath(frg_res[k], "/");
                        idx = atoi(idxs[0].c_str()); vn_idx = atoi(idxs[1].c_str());
                        idx--; vn_idx--;
                        tempMesh.indices.push_back(idx);
                        f.push_back(idx);
                        tempMesh.vertices[idx].normal = tempMesh.vertNormals[vn_idx];
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                    }
                }
                else if (vn_count == 0 && vt_count != 0) {
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitOBJpath(frg_res[k], "/");
                        idx = atoi(idxs[0].c_str()); vt_idx = atoi(idxs[1].c_str());
                        idx--; vt_idx--;
                        tempMesh.indices.push_back(idx);
                        f.push_back(idx);
                        tempMesh.vertices[idx].texUv = tempMesh.vertUVs[vt_idx];
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                    }
                }
                else if (vn_count != 0 && vt_count != 0) {
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitOBJpath(frg_res[k], "/");
                        idx = atoi(idxs[0].c_str()); vt_idx = atoi(idxs[1].c_str()); vn_idx = atoi(idxs[2].c_str());
                        idx--; vn_idx--; vt_idx--;
                        tempMesh.indices.push_back(idx);
                        f.push_back(idx);
                        tempMesh.vertices[idx].normal = tempMesh.vertNormals[vn_idx];
                        tempMesh.vertices[idx].texUv = tempMesh.vertUVs[vt_idx];
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                    }
                }
                tempMesh.faces.push_back(f);
            }
        }
        vertexNum += tempMesh.vertices.size();
        faceNum += tempMesh.faces.size();
        tempMesh.diffuseTextureIndex = getMeshTexture(*(path), "diffuse");
        tempMesh.specularTextureIndex = getMeshTexture(*(path), "specular");
        if (vn_count == 0) computeNormal(tempMesh);
        if (vt_count == 0) {
            for (int i = 0; i < tempMesh.vertices.size(); ++i) {
                tempMesh.vertices[i].texUv = Coord2D(0.0f, 0.0f);
            }
        }
        meshes.push_back(tempMesh);
        qDebug() << "Model Id:" << i + 1 << "vertex:" << v_count << "normal:" << vn_count << "texture:" << vt_count << "face:" << f_count;
    }
}

void Model::computeNormal(sigMesh& inMesh)
{
    std::vector<Vector3D> faceNormals;
    for (int i = 0; i < inMesh.indices.size(); i += 3) {
        Vector3D AB = (inMesh.vertices[(inMesh.indices[i + 1])]).worldPos - (inMesh.vertices[(inMesh.indices[i])]).worldPos;
        Vector3D AC = (inMesh.vertices[(inMesh.indices[i + 2])]).worldPos - (inMesh.vertices[(inMesh.indices[i])]).worldPos;
        Vector3D faceN = glm::normalize(glm::cross(AB, AC));
        faceNormals.push_back(faceN);
    }
    for (int i = 0; i < inMesh.vertices.size(); ++i) {
        std::vector<int> tempMs = inMesh.verToFace.at(i);
        Vector3D verNave(0.0, 0.0, 0.0);
        for (int j = 0; j < tempMs.size(); ++j) {
            verNave += faceNormals[tempMs[j]];
        }
        verNave /= tempMs.size();
        verNave = glm::normalize(verNave);
        inMesh.vertices[i].normal = verNave;
    }
}

int Model::getMeshTexture(QString path, std::string type)
{
    QString t_p = path;
    std::ifstream diffuse;
    t_p.chop(4);
    std::string t_ps = t_p.toStdString() + "_" + type + ".png";
    diffuse.open(t_ps, std::ifstream::in);
    if (diffuse.fail()) return -1;

    Texture texture;
    if (texture.getTexture(QString::fromStdString(t_ps)))
    {
        qDebug() << QString::fromStdString(t_ps);
        textureList.push_back(texture);
        return (int)textureList.size() - 1;
    }
    else return -1;
}


