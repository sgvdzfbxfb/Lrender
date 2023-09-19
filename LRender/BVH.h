#pragma once
#include <algorithm>
#include <cassert>
#include "triangle.h"

struct BVHBuildNode;
// BVHAccel Forward Declarations
struct BVHPrimitiveInfo;

// BVHAccel Declarations
inline int leafNodes, totalLeafNodes, totalPrimitives, interiorNodes;
class BVHAccel {

public:
    // BVHAccel Public Types
    enum class SplitMethod { NAIVE, SAH };

    // BVHAccel Public Methods
    BVHAccel(std::vector<BVHItem*> p, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);
    ~BVHAccel();

    Intersection Intersect(const Ray& ray) const;
    Intersection getIntersection(BVHBuildNode* node, const Ray& ray)const;
    BVHBuildNode* root = nullptr;

    // BVHAccel Private Methods
    BVHBuildNode* recursiveBuild(std::vector<BVHItem*>objects);

    // BVHAccel Private Data
    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<BVHItem*> primitives;

    void getSample(BVHBuildNode* node, float p, Intersection& pos, float& pdf);
    void Sample(Intersection& pos, float& pdf);
};

struct BVHBuildNode {
    Bounds3 bounds;
    BVHBuildNode* left;
    BVHBuildNode* right;
    BVHItem* object;
    float area = 0;

public:
    int splitAxis = 0, firstPrimOffset = 0, nPrimitives = 0;
    // BVHBuildNode Public Methods
    BVHBuildNode() {
        bounds = Bounds3();
        left = nullptr; right = nullptr;
        object = nullptr;
    }
};