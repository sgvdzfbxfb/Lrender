#ifndef MESH_H
#define MESH_H

#include <QString>
#include "texture.h"
#include "BVH.h"
#include "renderAPI.h"

bool rayTriangleIntersect(const Vector3D& v0, const Vector3D& v1, const Vector3D& v2, const Vector3D& orig, const Vector3D& dir, float& tnear, float& u, float& v);
inline Vector3D lerp(const Vector3D& a, const Vector3D& b, const float& t);

class sigMesh : public BVHItem {
public:
    std::vector<Vertex> vertices;
    std::vector<Triangle> faces;
    std::vector<Triangle> app_ani_faces;
    std::map<int, std::vector<int>> verToFace;
    std::map<int, std::vector<int>> faceToVer;
    std::vector<Vector3D> vertNormals;
    std::vector<Coord2D> vertUVs;
    std::vector<VectorI4D> vertJoints;
    std::vector<Vector4D> vertWeights;
    std::vector<int> diffuseIds;
    std::vector<int> specularIds;
    bool ifAnimation = false;

    std::vector<Texture> tList;
    Texture* m;

    float minX_sig{ FLT_MAX };
    float minY_sig{ FLT_MAX };
    float minZ_sig{ FLT_MAX };
    float maxX_sig{ FLT_MIN };
    float maxY_sig{ FLT_MIN };
    float maxZ_sig{ FLT_MIN };
    Bounds3 bounding_box;
    std::unique_ptr<uint32_t[]> vertexIndex;
    std::unique_ptr<Vector2D[]> stCoordinates;

    BVHAccel* bvh = nullptr;
    float area = 0;
    std::string sigMeshName = "";

    sigMesh(const QString& filename, std::vector<std::string>& texPaths, std::string& meshName, Texture* mt = new Texture());
    sigMesh(const sigMesh&);
    void computeNormal();
    void computeBVH();
    int getMeshTexture(std::string t_ps);
    void meshRender();

    bool intersect(const Ray& ray) { return true; }

    bool intersect(const Ray& ray, float& tnear, uint32_t& index) const
    {
        bool intersect = false;
        for (uint32_t k = 0; k < faces.size(); ++k) {
            const Vector3D& v0 = vertices[vertexIndex[k * 3]].worldPos;
            const Vector3D& v1 = vertices[vertexIndex[k * 3 + 1]].worldPos;;
            const Vector3D& v2 = vertices[vertexIndex[k * 3 + 2]].worldPos;;
            float t, u, v;
            if (rayTriangleIntersect(v0, v1, v2, ray.origin, ray.direction, t,
                u, v) &&
                t < tnear) {
                tnear = t;
                index = k;
                intersect |= true;
            }
        }

        return intersect;
    }

    Bounds3 getBounds() { return bounding_box; }

    void getSurfaceProperties(const Vector3D& P, const Vector3D& I, const uint32_t& index, const Vector2D& uv, Vector3D& N, Vector2D& st) const
    {
        const Vector3D& v0 = vertices[vertexIndex[index * 3]].worldPos;
        const Vector3D& v1 = vertices[vertexIndex[index * 3 + 1]].worldPos;
        const Vector3D& v2 = vertices[vertexIndex[index * 3 + 2]].worldPos;
        Vector3D e0 = normalize(v1 - v0);
        Vector3D e1 = normalize(v2 - v1);
        N = normalize(glm::cross(e0, e1));
        const Vector2D& st0 = stCoordinates[vertexIndex[index * 3]];
        const Vector2D& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
        const Vector2D& st2 = stCoordinates[vertexIndex[index * 3 + 2]];
        st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
    }

    Vector3D evalDiffuseColor(const Vector2D& st) const
    {
        float scale = 5;
        float pattern =
            (fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
        return lerp(Vector3D(0.815, 0.235, 0.031),
            Vector3D(0.937, 0.937, 0.231), pattern);
    }

    Intersection getIntersection(Ray ray)
    {
        Intersection intersec;

        if (bvh) {
            intersec = bvh->Intersect(ray);
        }

        return intersec;
    }

    void Sample(Intersection& pos, float& pdf) {
        bvh->Sample(pos, pdf);
        pos.emit_I = m->m_emission;
    }
    float getArea() {
        return area;
    }
    bool hasEmit() {
        if (glm::length(m->m_emission) > EPSILON) return true;
        else return false;
    }
};

#endif // MESH_H
