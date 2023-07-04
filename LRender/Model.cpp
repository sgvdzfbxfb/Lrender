#include "model.h"
#include <QDebug>

Model::Model(QStringList paths)
{
    loadModel(paths);
    modelCenter = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};
    skeleton.skeleton_load("D:/Code/lrender/LRender/model/kgirl/kgirl.ani");
    fTimeCounter.start();
}

void Model::modelRender()
{
    renderAPI::API().textureList = textureList;
    updateModelSkeleton((float)fTimeCounter.elapsed() / 1000.0);
    for(int i = 0; i < meshes.size(); i++) meshes.at(i).meshRender();
}

void Model::updateModelSkeleton(float ft)
{
    Skeleton skTemp = skeleton;
    std::vector<glm::mat4> joint_matrices;
    std::vector<glm::mat3> joint_n_matrices;
    if (skTemp.ske.joints.size() != 0) {
        skTemp.skeleton_update_joints(&(skTemp.ske), ft);
        joint_matrices = skTemp.ske.joint_matrices;
        joint_n_matrices = skTemp.ske.normal_matrices;
        qDebug() << ft << "                                 " << joint_matrices.size() << joint_n_matrices.size();
    }
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
                Triangle f;
                int idx, vn_idx, vt_idx;
                std::vector<std::string> frg_res = splitString(line, " ");
                if (vn_count == 0 && vt_count == 0) {
                    int x = 0;
                    std::vector<int> vers;
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                        idx = atoi(idxs.at(0).c_str());
                        idx--;
                        f.at(x) = tempMesh.vertices.at(idx);
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                        vers.push_back(idx);
                        x++;
                    }
                    tempMesh.faceToVer[tempMesh.faces.size()] = vers;
                }
                else if (vn_count != 0 && vt_count == 0) {
                    int x = 0;
                    std::vector<int> vers;
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                        idx = atoi(idxs.at(0).c_str()); vn_idx = atoi(idxs.at(1).c_str());
                        idx--; vn_idx--;
                        f.at(x) = tempMesh.vertices.at(idx);
                        f.at(x).normal = tempMesh.vertNormals.at(vn_idx);
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                        vers.push_back(idx);
                        x++;
                    }
                    tempMesh.faceToVer[tempMesh.faces.size()] = vers;
                }
                else if (vn_count == 0 && vt_count != 0) {
                    int x = 0;
                    std::vector<int> vers;
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                        idx = atoi(idxs.at(0).c_str()); vt_idx = atoi(idxs.at(1).c_str());
                        idx--; vt_idx--;
                        f.at(x) = tempMesh.vertices.at(idx);
                        f.at(x).texUv = tempMesh.vertUVs.at(vt_idx);
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                        vers.push_back(idx);
                        x++;
                    }
                    tempMesh.faceToVer[tempMesh.faces.size()] = vers;
                }
                else if (vn_count != 0 && vt_count != 0) {
                    int x = 0;
                    std::vector<int> vers;
                    for (int k = 1; k < frg_res.size(); ++k) {
                        std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                        idx = atoi(idxs.at(0).c_str()); vt_idx = atoi(idxs.at(1).c_str()); vn_idx = atoi(idxs.at(2).c_str());
                        idx--; vn_idx--; vt_idx--;
                        f.at(x) = tempMesh.vertices.at(idx);
                        f.at(x).normal = tempMesh.vertNormals.at(vn_idx);
                        f.at(x).texUv = tempMesh.vertUVs.at(vt_idx);
                        (tempMesh.verToFace[idx]).push_back(tempMesh.faces.size());
                        vers.push_back(idx);
                        x++;
                    }
                    tempMesh.faceToVer[tempMesh.faces.size()] = vers;
                }
                tempMesh.faces.push_back(f);
            }
        }
        vertexNum += tempMesh.vertices.size();
        faceNum += tempMesh.faces.size();
        std::vector<std::string> texPaths; std::string folderPath;
        std::vector<std::string> folderSp = splitString((*(path)).toStdString(), "/");
        for (int k = 0; k < folderSp.size() - 1; ++k) folderPath += folderSp.at(k) + "/";
        folderPath.pop_back();
        getAllImageFiles(folderPath, texPaths);
        for (int k = 0; k < texPaths.size(); ++k) {
            if (texPaths.at(k).find("diffuse") > 0 && texPaths.at(k).find("diffuse") < texPaths.at(k).length())
                tempMesh.diffuseIds.push_back(getMeshTexture(texPaths.at(k), "diffuse"));
            else if (texPaths.at(k).find("specular") > 0 && texPaths.at(k).find("specular") < texPaths.at(k).length())
                tempMesh.specularIds.push_back(getMeshTexture(texPaths.at(k), "specular"));
        }
        if (vn_count == 0) computeNormal(tempMesh);
        if (vt_count == 0) {
            for (int i = 0; i < tempMesh.vertices.size(); ++i) {
                tempMesh.vertices.at(i).texUv = Coord2D(0.0f, 0.0f);
            }
        }
        meshes.push_back(tempMesh);
        qDebug() << "Model Id:" << i + 1 << "vertex:" << v_count << "normal:" << vn_count << "texture:" << vt_count << "face:" << f_count;
    }
}

void Model::computeNormal(sigMesh& inMesh)
{
    std::vector<Vector3D> faceNormals;
    for (int i = 0; i < inMesh.faces.size(); i++) {
        Vector3D AB = inMesh.faces.at(i).at(1).worldPos - inMesh.faces.at(i).at(0).worldPos;
        Vector3D AC = inMesh.faces.at(i).at(2).worldPos - inMesh.faces.at(i).at(0).worldPos;
        Vector3D faceN = glm::normalize(glm::cross(AB, AC));
        faceNormals.push_back(faceN);
    }
    for (int i = 0; i < inMesh.vertices.size(); ++i) {
        std::vector<int> tempMs = inMesh.verToFace.at(i);
        Vector3D verNave(0.0, 0.0, 0.0);
        for (int j = 0; j < tempMs.size(); ++j) {
            verNave += faceNormals[tempMs.at(j)];
        }
        verNave /= tempMs.size();
        verNave = glm::normalize(verNave);
        inMesh.vertices.at(i).normal = verNave;
    }
    for (int i = 0; i < inMesh.faces.size(); ++i) {
        inMesh.faces.at(i).at(0).normal = inMesh.vertices[inMesh.faceToVer.at(i).at(0)].normal;
        inMesh.faces.at(i).at(1).normal = inMesh.vertices[inMesh.faceToVer.at(i).at(1)].normal;
        inMesh.faces.at(i).at(2).normal = inMesh.vertices[inMesh.faceToVer.at(i).at(2)].normal;
    }
}

int Model::getMeshTexture(std::string t_ps, std::string type)
{
    std::ifstream texStream;
    texStream.open(t_ps, std::ifstream::in);
    if (texStream.fail()) return -1;
    Texture texture;
    if (texture.getTexture(QString::fromStdString(t_ps)))
    {
        qDebug() << QString::fromStdString(t_ps);
        textureList.push_back(texture);
        return (int)textureList.size() - 1;
    }
    else return -1;
}
