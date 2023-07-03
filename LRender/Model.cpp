#include "model.h"
#include <QDebug>

using namespace std::filesystem;

std::vector<std::string> splitString(const std::string& str, const std::string& delim) {
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

// 检查一个路径是否是目录
bool checkIsDir(const std::string& dir) {
    if (!exists(dir)) {
        qDebug() << QString::fromStdString(dir) << "not exists. Please check.";
        return false;
    }
    directory_entry entry(dir);
    if (entry.is_directory())
        return true;
    return false;
}

// 搜索一个目录下所有的图像文件，以 png 结尾的文件
void getAllImageFiles(const std::string dir, std::vector<std::string>& files) {
    // 首先检查目录是否为空，以及是否是目录
    if (!checkIsDir(dir)) return;
    // 递归遍历所有的文件
    directory_iterator iters(dir);
    for (auto& iter : iters) {
        // 路径的拼接，基础知识还是不过关，只能用这个笨办法来了
        std::string file_path(dir);
        file_path += "/"; file_path += (iter.path().filename()).string();
        // 查看是否是目录，如果是目录则循环递归
        if (checkIsDir(file_path)) {
            getAllImageFiles(file_path, files);
        }
        else { //不是目录则检查后缀是否是图像
            std::string extension = (iter.path().extension()).string(); // 获取文件的后缀名
            // 可以扩展成你自己想要的文件类型来进行筛选, 比如加上.gif .bmp之类的
            if (extension == ".png") {
                files.push_back(file_path);
            }
        }
    }
}

Model::Model(QStringList paths)
{
    loadModel(paths);
    modelCenter = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};
}

void Model::modelRender()
{
    renderAPI::API().textureList = textureList;
    for(int i = 0; i < meshes.size(); i++)
        meshes.at(i).meshRender();
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
                        std::vector<std::string> idxs = splitString(frg_res[k], "/");
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
                        std::vector<std::string> idxs = splitString(frg_res[k], "/");
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
                        std::vector<std::string> idxs = splitString(frg_res[k], "/");
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
                        std::vector<std::string> idxs = splitString(frg_res[k], "/");
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
        for (int k = 0; k < folderSp.size() - 1; ++k) folderPath += folderSp[k] + "/";
        folderPath.pop_back();
        getAllImageFiles(folderPath, texPaths);
        for (int k = 0; k < texPaths.size(); ++k) {
            if (texPaths[k].find("diffuse") > 0 && texPaths[k].find("diffuse") < texPaths[k].length())
                tempMesh.diffuseIds.push_back(getMeshTexture(texPaths[k], "diffuse"));
            else if (texPaths[k].find("specular") > 0 && texPaths[k].find("specular") < texPaths[k].length())
                tempMesh.specularIds.push_back(getMeshTexture(texPaths[k], "specular"));
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

void Model::loadSkyBox(std::string skyPath)
{
    std::vector<Vertex> skyBoxVers;
    Vertex Ver1; Ver1.worldPos = Coord3D(5.0, 5.0, -5.0); skyBoxVers.push_back(Ver1);
    Vertex Ver2; Ver2.worldPos = Coord3D(5.0, -5.0, -5.0); skyBoxVers.push_back(Ver2);
    Vertex Ver3; Ver3.worldPos = Coord3D(-5.0, -5.0, -5.0); skyBoxVers.push_back(Ver3);
    Vertex Ver4; Ver4.worldPos = Coord3D(-5.0, 5.0, -5.0); skyBoxVers.push_back(Ver4);
    Vertex Ver5; Ver5.worldPos = Coord3D(5.0, 5.0, 5.0); skyBoxVers.push_back(Ver5);
    Vertex Ver6; Ver6.worldPos = Coord3D(5.0, -5.0, 5.0); skyBoxVers.push_back(Ver6);
    Vertex Ver7; Ver7.worldPos = Coord3D(-5.0, -5.0, 5.0); skyBoxVers.push_back(Ver7);
    Vertex Ver8; Ver8.worldPos = Coord3D(-5.0, 5.0, 5.0); skyBoxVers.push_back(Ver8);

    Triangle face11{ skyBoxVers.at(1), skyBoxVers.at(2), skyBoxVers.at(3) };
    face11.at(0).texUv = Coord2D(0.0, 0.0); face11.at(1).texUv = Coord2D(1.0, 0.0); face11.at(2).texUv = Coord2D(1.0, 1.0);
    SkyBoxFaces.push_back(face11);
    Triangle face12{ skyBoxVers.at(1), skyBoxVers.at(3), skyBoxVers.at(0) };
    face12.at(0).texUv = Coord2D(0.0, 0.0); face12.at(1).texUv = Coord2D(1.0, 1.0); face12.at(2).texUv = Coord2D(0.0, 1.0);
    SkyBoxFaces.push_back(face12);

    Triangle face21{ skyBoxVers.at(2), skyBoxVers.at(6), skyBoxVers.at(7) };
    face21.at(0).texUv = Coord2D(0.0, 0.0); face21.at(1).texUv = Coord2D(1.0, 0.0); face21.at(2).texUv = Coord2D(1.0, 1.0);
    SkyBoxFaces.push_back(face21);
    Triangle face22{ skyBoxVers.at(2), skyBoxVers.at(7), skyBoxVers.at(3) };
    face22.at(0).texUv = Coord2D(0.0, 0.0); face22.at(1).texUv = Coord2D(1.0, 1.0); face22.at(2).texUv = Coord2D(0.0, 1.0);
    SkyBoxFaces.push_back(face22);

    Triangle face31{ skyBoxVers.at(6), skyBoxVers.at(5), skyBoxVers.at(4) };
    face31.at(0).texUv = Coord2D(0.0, 0.0); face31.at(1).texUv = Coord2D(1.0, 0.0); face31.at(2).texUv = Coord2D(1.0, 1.0);
    SkyBoxFaces.push_back(face31);
    Triangle face32{ skyBoxVers.at(6), skyBoxVers.at(4), skyBoxVers.at(7) };
    face32.at(0).texUv = Coord2D(0.0, 0.0); face32.at(1).texUv = Coord2D(1.0, 1.0); face32.at(2).texUv = Coord2D(0.0, 1.0);
    SkyBoxFaces.push_back(face32);

    Triangle face41{ skyBoxVers.at(5), skyBoxVers.at(1), skyBoxVers.at(0) };
    face41.at(0).texUv = Coord2D(0.0, 0.0); face41.at(1).texUv = Coord2D(1.0, 0.0); face41.at(2).texUv = Coord2D(1.0, 1.0);
    SkyBoxFaces.push_back(face41);
    Triangle face42{ skyBoxVers.at(5), skyBoxVers.at(0), skyBoxVers.at(4) };
    face42.at(0).texUv = Coord2D(0.0, 0.0); face42.at(1).texUv = Coord2D(1.0, 1.0); face42.at(2).texUv = Coord2D(0.0, 1.0);
    SkyBoxFaces.push_back(face42);

    Triangle face51{ skyBoxVers.at(7), skyBoxVers.at(4), skyBoxVers.at(0) };
    face51.at(0).texUv = Coord2D(0.0, 0.0); face51.at(1).texUv = Coord2D(1.0, 0.0); face51.at(2).texUv = Coord2D(1.0, 1.0);
    SkyBoxFaces.push_back(face51);
    Triangle face52{ skyBoxVers.at(7), skyBoxVers.at(0), skyBoxVers.at(3) };
    face52.at(0).texUv = Coord2D(0.0, 0.0); face52.at(1).texUv = Coord2D(1.0, 1.0); face52.at(2).texUv = Coord2D(0.0, 1.0);
    SkyBoxFaces.push_back(face52);

    Triangle face61{ skyBoxVers.at(2), skyBoxVers.at(1), skyBoxVers.at(5) };
    face61.at(0).texUv = Coord2D(0.0, 0.0); face61.at(1).texUv = Coord2D(1.0, 0.0); face61.at(2).texUv = Coord2D(1.0, 1.0);
    SkyBoxFaces.push_back(face61);
    Triangle face62{ skyBoxVers.at(2), skyBoxVers.at(5), skyBoxVers.at(6) };
    face62.at(0).texUv = Coord2D(0.0, 0.0); face62.at(1).texUv = Coord2D(1.0, 1.0); face62.at(2).texUv = Coord2D(0.0, 1.0);
    SkyBoxFaces.push_back(face62);

    std::vector<std::string> skyPaths;
    getAllImageFiles(skyPath, skyPaths);

    for (int i = 0; i < skyPaths.size(); ++i) {
        Texture texture;
        if (texture.getTexture(QString::fromStdString(skyPaths.at(i)))) {
            cubeMapSkyBox.push_back(texture);
            qDebug() << QString::fromStdString(skyPaths.at(i));
        }
    }
}

std::vector<Texture> Model::getSkyBox()
{
    return cubeMapSkyBox;
}
