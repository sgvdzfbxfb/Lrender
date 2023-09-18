#pragma once
#include "triangle.h"
#include "BVH.h"

class CornellBoxScene {
public:

	void initCornellBox();
    Intersection intersect(const Ray& ray) const;
    BVHAccel* bvh;
    void buildBVH();
    Vector3D castRay(const Ray& ray, int depth) const;
    void sampleLight(Intersection& pos, float& pdf) const;
    bool trace(const Ray& ray, const std::vector<BVHItem*>& objects, float& tNear, uint32_t& index, BVHItem** hitObject);
};