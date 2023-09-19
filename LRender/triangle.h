#pragma once
#include "lrenderBasicCore.h"
#include "texture.h"

class BVHItem;
class Texture;
struct Intersection
{
    Intersection() {
        happened = false;
        coords = Vector3D();
        normal = Vector3D();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }
    bool happened;
    Vector3D coords;
    Vector3D tcoords;
    Vector3D normal;
    Vector3D emit_I;
    double distance;
    BVHItem* obj;
    Texture* m;
};

class Bounds3
{
public:
    Vector3D pMin = Vector3D(0.0, 0.0, 0.0);
    Vector3D pMax = Vector3D(0.0, 0.0, 0.0);// two points to specify the bounding box
    Bounds3();
    Bounds3(const Vector3D p);
    Bounds3(const Vector3D p1, const Vector3D p2);

    Vector3D Diagonal() const;
    int maxExtent() const;

    double SurfaceArea() const;

    Vector3D Centroid();
    Bounds3 Intersect(const Bounds3& b);

    Vector3D Offset(const Vector3D& p) const;

    bool Overlaps(const Bounds3& b1, const Bounds3& b2);

    bool Inside(const Vector3D& p, const Bounds3& b);
    inline const Vector3D& operator[](int i) const;

    bool IntersectP(const Ray& ray, const Vector3D& invDir, const std::array<int, 3>& dirisNeg) const;
};

class BVHItem
{
public:
    BVHItem() {}
    virtual ~BVHItem() {}
    virtual bool intersect(const Ray& ray) = 0;
    virtual bool intersect(const Ray& ray, float&, uint32_t&) const = 0;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual void getSurfaceProperties(const Vector3D&, const Vector3D&, const uint32_t&, const Vector2D&, Vector3D&, Vector2D&) const = 0;
    virtual Vector3D evalDiffuseColor(const Vector2D&) const = 0;
    virtual Bounds3 getBounds() = 0;
    virtual float getArea() = 0;
    virtual void Sample(Intersection& pos, float& pdf) = 0;
    virtual bool hasEmit() = 0;
};

class Texture;
class Triangle : public BVHItem
{
public:
    Vertex v0, v1, v2; // vertices A, B ,C , counter-clockwise order
    Vector3D e1, e2;     // 2 edges v1-v0, v2-v0;
    Vector3D normal;
    float area;
    Texture* m;

    Triangle(Vertex _v0, Vertex _v1, Vertex _v2, Texture* _m = nullptr);
    bool intersect(const Ray& ray) override;
    bool intersect(const Ray& ray, float& tnear, uint32_t& index) const override;
    Intersection getIntersection(Ray ray) override;
    void getSurfaceProperties(const Vector3D& P, const Vector3D& I, const uint32_t& index, const Vector2D& uv, Vector3D& N, Vector2D& st) const override;
    Vector3D evalDiffuseColor(const Vector2D&) const override;
    Bounds3 getBounds() override;
    void Sample(Intersection& pos, float& pdf);
    float getArea();
    bool hasEmit();
};