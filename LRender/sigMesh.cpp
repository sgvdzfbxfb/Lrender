#include "sigMesh.h"
#include <QDebug>

bool rayTriangleIntersect(const Vector3D& v0, const Vector3D& v1, const Vector3D& v2, const Vector3D& orig, const Vector3D& dir, float& tnear, float& u, float& v)
{
    Vector3D edge1 = v1 - v0;
    Vector3D edge2 = v2 - v0;
    Vector3D pvec = glm::cross(dir, edge2);
    float det = glm::dot(edge1, pvec);
    if (det == 0 || det < 0)
        return false;

    Vector3D tvec = orig - v0;
    u = glm::dot(tvec, pvec);
    if (u < 0 || u > det)
        return false;

    Vector3D qvec = glm::cross(tvec, edge1);
    v = glm::dot(dir, qvec);
    if (v < 0 || u + v > det)
        return false;

    float invDet = 1 / det;

    tnear = glm::dot(edge2, qvec) * invDet;
    u *= invDet;
    v *= invDet;

    return true;
}
inline Vector3D lerp(const Vector3D& a, const Vector3D& b, const float& t)
{
    return a * (1 - t) + b * t;
}

sigMesh::sigMesh(const QString& filename, std::vector<std::string>& texPaths, std::string& meshName, Texture* mt) {
    area = 0;
    m = mt;
    sigMeshName = meshName;
    std::ifstream in, in_forCount;
    in.open(filename.toStdString(), std::ifstream::in);
    in_forCount.open(filename.toStdString(), std::ifstream::in);
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

            minX_sig = std::min(minX_sig, v_p.x);
            minY_sig = std::min(minY_sig, v_p.y);
            minZ_sig = std::min(minZ_sig, v_p.z);
            maxX_sig = std::max(maxX_sig, v_p.x);
            maxY_sig = std::max(maxY_sig, v_p.y);
            maxZ_sig = std::max(maxZ_sig, v_p.z);
            ver.worldPos = v_p;
            vertices.push_back(ver);
            v_count++;
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vector3D n;
            for (int j = 0; j < 3; j++) iss >> n[j];
            vertNormals.push_back(n);
            vn_count++;
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Coord2D uv;
            for (int j = 0; j < 2; j++) iss >> uv[j];
            vertUVs.push_back(uv);
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
            vertJoints.push_back(joint);
            v_joint++;
        }
        else if (!line.compare(0, 12, "# ext.weight")) {
            std::string t1, t2;
            iss >> t1 >> t2;
            Vector4D weight;
            for (int j = 0; j < 4; j++) iss >> weight[j];
            vertWeights.push_back(weight);
            v_weight++;
        }
    }
    while (!in.eof()) {
        std::getline(in, line);
        if (!line.compare(0, 2, "f ")) {
            std::array<Vertex, 3> f;
            int idx, vn_idx, vt_idx;
            std::vector<std::string> frg_res = splitString(line, " ");
            if (vn_count == 0 && vt_count == 0) {
                int x = 0;
                std::vector<int> vers;
                for (int k = 1; k < frg_res.size(); ++k) {
                    std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                    idx = atoi(idxs.at(0).c_str());
                    idx--;
                    f.at(x) = vertices.at(idx);
                    (verToFace[idx]).push_back(faces.size());
                    vers.push_back(idx);
                    x++;
                }
                faceToVer[faces.size()] = vers;
            }
            else if (vn_count != 0 && vt_count == 0) {
                int x = 0;
                std::vector<int> vers;
                for (int k = 1; k < frg_res.size(); ++k) {
                    std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                    idx = atoi(idxs.at(0).c_str()); vn_idx = atoi(idxs.at(1).c_str());
                    idx--; vn_idx--;
                    f.at(x) = vertices.at(idx);
                    f.at(x).normal = vertNormals.at(vn_idx);
                    (verToFace[idx]).push_back(faces.size());
                    vers.push_back(idx);
                    x++;
                }
                faceToVer[faces.size()] = vers;
            }
            else if (vn_count == 0 && vt_count != 0) {
                int x = 0;
                std::vector<int> vers;
                for (int k = 1; k < frg_res.size(); ++k) {
                    std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                    idx = atoi(idxs.at(0).c_str()); vt_idx = atoi(idxs.at(1).c_str());
                    idx--; vt_idx--;
                    f.at(x) = vertices.at(idx);
                    f.at(x).texUv = vertUVs.at(vt_idx);
                    (verToFace[idx]).push_back(faces.size());
                    vers.push_back(idx);
                    x++;
                }
                faceToVer[faces.size()] = vers;
            }
            else if (vn_count != 0 && vt_count != 0) {
                int x = 0;
                std::vector<int> vers;
                for (int k = 1; k < frg_res.size(); ++k) {
                    std::vector<std::string> idxs = splitString(frg_res.at(k), "/");
                    idx = atoi(idxs.at(0).c_str()); vt_idx = atoi(idxs.at(1).c_str()); vn_idx = atoi(idxs.at(2).c_str());
                    idx--; vn_idx--; vt_idx--;
                    f.at(x) = vertices.at(idx);
                    f.at(x).normal = vertNormals.at(vn_idx);
                    f.at(x).texUv = vertUVs.at(vt_idx);
                    (verToFace[idx]).push_back(faces.size());
                    vers.push_back(idx);
                    x++;
                }
                faceToVer[faces.size()] = vers;
            }
            Triangle tri_f(f.at(0), f.at(1), f.at(2), mt);
            faces.push_back(tri_f);
        }
    }

    for (int k = 0; k < texPaths.size(); ++k) {
        if (texPaths.at(k).find(meshName) > 0 && texPaths.at(k).find(meshName) < texPaths.at(k).length()) {
            if (texPaths.at(k).find("diffuse") > 0 && texPaths.at(k).find("diffuse") < texPaths.at(k).length())
                diffuseIds.push_back(getMeshTexture(texPaths.at(k)));
            else if (texPaths.at(k).find("specular") > 0 && texPaths.at(k).find("specular") < texPaths.at(k).length())
                specularIds.push_back(getMeshTexture(texPaths.at(k)));
        }
    }
    if (vn_count == 0) computeNormal();
    if (vt_count == 0) {
        for (int j = 0; j < vertices.size(); ++j) {
            vertices.at(j).texUv = Coord2D(0.0f, 0.0f);
        }
    }
    if (v_joint == v_count) {
        for (int j = 0; j < faces.size(); ++j) {
            faces.at(j).v0.joint = vertJoints[faceToVer.at(j).at(0)];
            faces.at(j).v1.joint = vertJoints[faceToVer.at(j).at(1)];
            faces.at(j).v2.joint = vertJoints[faceToVer.at(j).at(2)];

            faces.at(j).v0.weight = vertWeights[faceToVer.at(j).at(0)];
            faces.at(j).v1.weight = vertWeights[faceToVer.at(j).at(1)];
            faces.at(j).v2.weight = vertWeights[faceToVer.at(j).at(2)];
        }
        ifAnimation = true;
    }
    if (diffuseIds.size() > 0) *m = tList.at(diffuseIds.at(0));
    qDebug() << "Model Name:" << QString::fromStdString(meshName);
    qDebug() << "vertex:" << v_count << "normal:" << vn_count << "texture:" << vt_count << "face:" << f_count;
    qDebug() << "joint:" << v_joint << "weight:" << v_weight << "\n";
}
sigMesh::sigMesh(const sigMesh& mesh): m(mesh.m) {}

void sigMesh::computeBVH() {
    minX_sig = FLT_MAX; minY_sig = FLT_MAX; minZ_sig = FLT_MAX;
    maxX_sig = FLT_MIN; maxY_sig = FLT_MIN; maxZ_sig = FLT_MIN;
    std::vector<BVHItem*> ptrs;
    for (auto& tri : app_ani_faces) {
        minX_sig = std::min(minX_sig, tri.v0.worldPos.x); minY_sig = std::min(minY_sig, tri.v0.worldPos.y); minZ_sig = std::min(minZ_sig, tri.v0.worldPos.z);
        maxX_sig = std::max(maxX_sig, tri.v0.worldPos.x); maxY_sig = std::max(maxY_sig, tri.v0.worldPos.y); maxZ_sig = std::max(maxZ_sig, tri.v0.worldPos.z);

        minX_sig = std::min(minX_sig, tri.v1.worldPos.x); minY_sig = std::min(minY_sig, tri.v1.worldPos.y); minZ_sig = std::min(minZ_sig, tri.v1.worldPos.z);
        maxX_sig = std::max(maxX_sig, tri.v1.worldPos.x); maxY_sig = std::max(maxY_sig, tri.v1.worldPos.y); maxZ_sig = std::max(maxZ_sig, tri.v1.worldPos.z);

        minX_sig = std::min(minX_sig, tri.v2.worldPos.x); minY_sig = std::min(minY_sig, tri.v2.worldPos.y); minZ_sig = std::min(minZ_sig, tri.v2.worldPos.z);
        maxX_sig = std::max(maxX_sig, tri.v2.worldPos.x); maxY_sig = std::max(maxY_sig, tri.v2.worldPos.y); maxZ_sig = std::max(maxZ_sig, tri.v2.worldPos.z);

        ptrs.push_back(&tri);
        area += tri.area;
    }
    Vector3D min_vert(minX_sig, minY_sig, minZ_sig);
    Vector3D max_vert(maxX_sig, maxY_sig, maxZ_sig);
    bounding_box = Bounds3(min_vert, max_vert);
    qDebug() << "meshName: " << QString::fromStdString(sigMeshName);
    bvh = new BVHAccel(ptrs);
}

void sigMesh::computeNormal()
{
    std::vector<Vector3D> faceNormals;
    for (int i = 0; i < faces.size(); i++) {
        Vector3D AB = faces.at(i).v1.worldPos - faces.at(i).v0.worldPos;
        Vector3D AC = faces.at(i).v2.worldPos - faces.at(i).v0.worldPos;
        Vector3D faceN = glm::normalize(glm::cross(AB, AC));
        faceNormals.push_back(faceN);
    }
    for (int i = 0; i < vertices.size(); ++i) {
        std::vector<int> tempMs = verToFace.at(i);
        Vector3D verNave(0.0, 0.0, 0.0);
        for (int j = 0; j < tempMs.size(); ++j) {
            verNave += faceNormals[tempMs.at(j)];
        }
        verNave /= tempMs.size();
        verNave = glm::normalize(verNave);
        vertices.at(i).normal = verNave;
    }
    for (int i = 0; i < faces.size(); ++i) {
        faces.at(i).v0.normal = vertices[faceToVer.at(i).at(0)].normal;
        faces.at(i).v1.normal = vertices[faceToVer.at(i).at(1)].normal;
        faces.at(i).v2.normal = vertices[faceToVer.at(i).at(2)].normal;
    }
}

int sigMesh::getMeshTexture(std::string t_ps)
{
    std::ifstream texStream;
    texStream.open(t_ps, std::ifstream::in);
    if (texStream.fail()) return -1;
    Texture texture;
    if (texture.getTexture(QString::fromStdString(t_ps)))
    {
        qDebug() << QString::fromStdString(t_ps);
        tList.push_back(texture);
        return (int)tList.size() - 1;
    }
    else return -1;
}

void sigMesh::meshRender() {
    renderAPI::API().textureList = tList;
    renderAPI::API().faces = faces;
    renderAPI::API().shader->material.diffuse = diffuseIds;
    renderAPI::API().shader->material.specular = specularIds;
    renderAPI::API().render(ifAnimation);
    app_ani_faces = renderAPI::API().faces;
}
