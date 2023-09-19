#pragma once
#include <mutex>
#include <thread>
#include "triangle.h"
#include "camera.h"
#include "model.h"
#include "BVH.h"

class CornellBoxScene {
public:
    std::vector<BVHItem*> boxModels;
    std::vector<std::vector<Triangle>> input_faces;
    float RussianRoulette = 0.8;
    Vector3D backgroundColor = Vector3D(0.0, 0.0, 0.0);
    int width_cornellBox, height_cornellBox;
    Camera camera;
    Frame frame;
    BVHAccel* bvh = nullptr;

    int castrcount = 0;

    CornellBoxScene(Model* input_model, int wid_p, int hei_p, Color bkColor);
    Intersection intersect(const Ray& ray) const;
    void buildBVH();
    Vector3D castRay(const Ray& ray, int depth);
    void sampleLight(Intersection& pos, float& pdf) const;
    bool trace(const Ray& ray, const std::vector<BVHItem*>& objects, float& tNear, uint32_t& index, BVHItem** hitObject);
    void cornellBoxRender();
};