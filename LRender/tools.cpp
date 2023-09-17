#include "tools.h"

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

// ���һ��·���Ƿ���Ŀ¼
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

// ����һ��Ŀ¼�����е�ͼ���ļ����� png ��β���ļ�
void getAllTypeFiles(const std::string dir, std::vector<std::string>& files, std::string type) {
    // ���ȼ��Ŀ¼�Ƿ�Ϊ�գ��Լ��Ƿ���Ŀ¼
    if (!checkIsDir(dir)) return;
    // �ݹ�������е��ļ�
    directory_iterator iters(dir);
    for (auto& iter : iters) {
        // ·����ƴ�ӣ�����֪ʶ���ǲ����أ�ֻ����������취����
        std::string file_path(dir);
        file_path += "/"; file_path += (iter.path().filename()).string();
        // �鿴�Ƿ���Ŀ¼�������Ŀ¼��ѭ���ݹ�
        if (checkIsDir(file_path)) {
            getAllTypeFiles(file_path, files, type);
        }
        else { //����Ŀ¼�����׺�Ƿ���ͼ��
            std::string extension = (iter.path().extension()).string(); // ��ȡ�ļ��ĺ�׺��
            // ������չ�����Լ���Ҫ���ļ�����������ɸѡ, �������.gif .bmp֮���
            if (extension == "." + type) {
                files.push_back(file_path);
            }
        }
    }
}

Coord3D interpolate(float alpha, float beta, float gamma, const Coord3D& vert1, const Coord3D& vert2, const Coord3D& vert3, float weight)
{
    return (alpha * vert1 + beta * vert2 + gamma * vert3) / weight;
}

Coord2D interpolate(float alpha, float beta, float gamma, const Coord2D& vert1, const Coord2D& vert2, const Coord2D& vert3, float weight)
{
    auto u = (alpha * vert1[0] + beta * vert2[0] + gamma * vert3[0]);
    auto v = (alpha * vert1[1] + beta * vert2[1] + gamma * vert3[1]);

    u /= weight;
    v /= weight;

    return Coord2D(u, v);
}

std::vector<Triangle> constructTriangle(std::vector<Vertex> vertexList)
{
    std::vector<Triangle> res;
    for (int i = 0; i < vertexList.size() - 2; i++)
    {
        int k = (i + 1) % vertexList.size();
        int m = (i + 2) % vertexList.size();
        Triangle tri{ vertexList.at(0), vertexList.at(k), vertexList.at(m) };
        res.push_back(tri);
    }
    return res;
}

Fragment interpolationFragment(int x, int y, float z, Triangle& tri, Vector3D& barycentric)
{
    Fragment frag;
    frag.screenPos.x = x;
    frag.screenPos.y = y;
    frag.zValue = z;

    Vector3D bc_corrected = { 0, 0, 0 };
    bc_corrected[0] = barycentric[0] / tri.v0.clipPos.w;
    bc_corrected[1] = barycentric[1] / tri.v1.clipPos.w;
    bc_corrected[2] = barycentric[2] / tri.v2.clipPos.w;
    float Z_n = 1. / (bc_corrected[0] + bc_corrected[1] + bc_corrected[2]);
    for (int i = 0; i < 3; i++) bc_corrected[i] *= Z_n;

    frag.worldPos += tri.v0.worldPos * bc_corrected[0];
    frag.worldPos += tri.v1.worldPos * bc_corrected[1];
    frag.worldPos += tri.v2.worldPos * bc_corrected[2];
    frag.normal += tri.v0.normal * bc_corrected[0];
    frag.normal += tri.v1.normal * bc_corrected[1];
    frag.normal += tri.v2.normal * bc_corrected[2];
    frag.texUv += tri.v0.texUv * bc_corrected[0];
    frag.texUv += tri.v1.texUv * bc_corrected[1];
    frag.texUv += tri.v2.texUv * bc_corrected[2];

    return frag;
}