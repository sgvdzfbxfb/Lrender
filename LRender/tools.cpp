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

// renderAPI related function
static inline bool isInTriangle(Vector3D bc_screen)
{
    bool flag = true;
    if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) flag = false;
    return flag;
}

template<class T>
static inline T calculateInterpolation(T a, T b, float alpha)
{
    return a * (1 - alpha) + b * alpha;
}

static inline CoordI2D calculateInterpolation(CoordI2D a, CoordI2D b, float alpha)
{
    CoordI2D res;
    res.x = static_cast<int>(a.x * (1 - alpha) + b.x * alpha + 0.5f);
    res.y = static_cast<int>(a.y * (1 - alpha) + b.y * alpha + 0.5f);
    return res;
}

static inline Vertex calculateInterpolation(Vertex a, Vertex b, float alpha)
{
    Vertex res;
    res.clipPos = calculateInterpolation(a.clipPos, b.clipPos, alpha);
    res.worldPos = calculateInterpolation(a.worldPos, b.worldPos, alpha);
    res.normal = calculateInterpolation(a.normal, b.normal, alpha);
    res.texUv = calculateInterpolation(a.texUv, b.texUv, alpha);
    return res;
}

template<class T>
static inline float calculateDistance(T point, T border)
{
    return glm::dot(point, border);
}

template<class T, size_t N>
static inline std::bitset<N> getClipCode(T point, std::array<T, N>& clip)
{
    std::bitset<N> res;
    for (int i = 0; i < N; i++)
        if (calculateDistance(point, clip.at(i)) < 0) res.set(i, 1);
    return res;
}

static Coord3D interpolate(float alpha, float beta, float gamma, const Coord3D& vert1, const Coord3D& vert2, const Coord3D& vert3, float weight)
{
    return (alpha * vert1 + beta * vert2 + gamma * vert3) / weight;
}

static Coord2D interpolate(float alpha, float beta, float gamma, const Coord2D& vert1, const Coord2D& vert2, const Coord2D& vert3, float weight)
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
    for (int i = 0; i < 3; i++) bc_corrected[i] = barycentric[i] / tri[i].clipPos.w;
    float Z_n = 1. / (bc_corrected[0] + bc_corrected[1] + bc_corrected[2]);
    for (int i = 0; i < 3; i++) bc_corrected[i] *= Z_n;

    for (int i = 0; i < 3; i++) frag.worldPos += tri[i].worldPos * bc_corrected[i];
    for (int i = 0; i < 3; i++) frag.normal += tri[i].normal * bc_corrected[i];
    for (int i = 0; i < 3; i++) frag.texUv += tri[i].texUv * bc_corrected[i];

    return frag;
}