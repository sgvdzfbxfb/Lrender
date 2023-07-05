#include "model.h"
#include <QDebug>

Model::Model(QStringList paths)
{
    QList<QString>::Iterator path = paths.begin(), itend = paths.end();
    for (int i = 0; path != itend; path++, i++) {
        std::vector<std::string> folderSp = splitString((*(path)).toStdString(), "/");
        if (i == 0) {
            for (int k = 0; k < folderSp.size() - 1; ++k) folderPath += folderSp.at(k) + "/";
            folderPath.pop_back();
        }
        std::string meshName = folderSp.at(folderSp.size() - 1).substr(0, folderSp.at(folderSp.size() - 1).length() - 4);
        meshNames.push_back(meshName);
        qDebug() << "meshNames" << QString::fromStdString(meshName);
    }

    loadModel(paths);
    modelCenter = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};

    ifModelAnimation = skeleton.skeleton_load(folderPath);
    if (ifModelAnimation) fTimeCounter.start();
}

void Model::modelRender()
{
    renderAPI::API().textureList = textureList;
    if (ifModelAnimation) updateModelSkeleton((float)fTimeCounter.elapsed() / 1000.0);
    for(int i = 0; i < meshes.size(); i++) meshes.at(i).meshRender();
}

void Model::updateModelSkeleton(float ft)
{
    Skeleton skTemp = skeleton;
    if (skTemp.ske.joints.size() != 0) {
        skTemp.skeleton_update_joints(&(skTemp.ske), ft);
        renderAPI::API().shader->joint_matrices = skTemp.ske.joint_matrices;
        renderAPI::API().shader->joint_n_matrices = skTemp.ske.normal_matrices;
        qDebug() << ft << "                                 " << renderAPI::API().shader->joint_matrices.size() << renderAPI::API().shader->joint_n_matrices.size();

    }
}

void Model::loadModel(QStringList paths)
{
    QList<QString>::Iterator path = paths.begin(), itend = paths.end();
    std::vector<std::string> texPaths;
    getAllTypeFiles(folderPath, texPaths, "png");
    for (int i = 0; path != itend; path++, i++) {
        sigMesh tempMesh;
        std::ifstream in, in_forCount;
        in.open(path->toStdString(), std::ifstream::in);
        in_forCount.open(path->toStdString(), std::ifstream::in);
        if (in.fail() || in_forCount.fail()) return;
        std::string line;
        int v_count = 0, vn_count = 0, vt_count = 0, f_count = 0;
        int v_joint = 0, v_weight = 0;
        while (!in_forCount.eof()) {
            std::getline(in_forCount, line);
            std::istringstream iss(line.c_str());
            char trash;
            if (!line.compare(0, 2, "v ")) {
                iss >> trash;
                Vertex ver;
                Vector3D v_p;
                for (int j = 0; j < 3; j++) iss >> v_p[j];

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
                for (int j = 0; j < 3; j++) iss >> n[j];
                tempMesh.vertNormals.push_back(n);
                vn_count++;
            }
            else if (!line.compare(0, 3, "vt ")) {
                iss >> trash >> trash;
                Coord2D uv;
                for (int j = 0; j < 2; j++) iss >> uv[j];
                tempMesh.vertUVs.push_back(uv);
                vt_count++;
            }
            else if (!line.compare(0, 2, "f ")) {
                f_count++;
            }
            else if (!line.compare(0, 11, "# ext.joint")) {
                std::string t1, t2;
                iss >> t1 >> t2;
                VectorI4D joint;
                for (int j = 0; j < 4; j++) iss >> joint[j];
                tempMesh.vertJoints.push_back(joint);
                v_joint++;
            }
            else if (!line.compare(0, 12, "# ext.weight")) {
                std::string t1, t2;
                iss >> t1 >> t2;
                Vector4D weight;
                for (int j = 0; j < 4; j++) iss >> weight[j];
                tempMesh.vertWeights.push_back(weight);
                v_weight++;
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
        
        for (int k = 0; k < texPaths.size(); ++k) {
            if (texPaths.at(k).find(meshNames.at(i)) > 0 && texPaths.at(k).find(meshNames.at(i)) < texPaths.at(k).length()) {
                if (texPaths.at(k).find("diffuse") > 0 && texPaths.at(k).find("diffuse") < texPaths.at(k).length())
                    tempMesh.diffuseIds.push_back(getMeshTexture(texPaths.at(k), "diffuse"));
                else if (texPaths.at(k).find("specular") > 0 && texPaths.at(k).find("specular") < texPaths.at(k).length())
                    tempMesh.specularIds.push_back(getMeshTexture(texPaths.at(k), "specular"));
            }
        }
        if (vn_count == 0) computeNormal(tempMesh);
        if (vt_count == 0) {
            for (int j = 0; j < tempMesh.vertices.size(); ++j) {
                tempMesh.vertices.at(i).texUv = Coord2D(0.0f, 0.0f);
            }
        }
        if (v_joint == v_count) {
            for (int j = 0; j < tempMesh.faces.size(); ++j) {
                tempMesh.faces.at(j).at(0).joint = tempMesh.vertJoints[tempMesh.faceToVer.at(j).at(0)];
                tempMesh.faces.at(j).at(1).joint = tempMesh.vertJoints[tempMesh.faceToVer.at(j).at(1)];
                tempMesh.faces.at(j).at(2).joint = tempMesh.vertJoints[tempMesh.faceToVer.at(j).at(2)];

                tempMesh.faces.at(j).at(0).weight = tempMesh.vertWeights[tempMesh.faceToVer.at(j).at(0)];
                tempMesh.faces.at(j).at(1).weight = tempMesh.vertWeights[tempMesh.faceToVer.at(j).at(1)];
                tempMesh.faces.at(j).at(2).weight = tempMesh.vertWeights[tempMesh.faceToVer.at(j).at(2)];
            }
            tempMesh.ifAnimation = true;
            ifModelAnimation = true;
        }
        meshes.push_back(tempMesh);
        qDebug() << "Model Id:" << i + 1 << "Model Name:" << QString::fromStdString(meshNames.at(i));
        qDebug() << "vertex:" << v_count << "normal:" << vn_count << "texture:" << vt_count << "face:" << f_count;
        qDebug() << "joint:" << v_joint << "weight:" << v_weight << "\n";
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
