#include "cornellBoxScene.h"

CornellBoxScene::CornellBoxScene(Model* input_model, int wid_p, int hei_p, Color bkColor)
    :width_cornellBox(wid_p), height_cornellBox(hei_p), camera((float)wid_p / hei_p, 100.f), frame(width_cornellBox, height_cornellBox) {
	QString prePath = "./cornellbox/";
	QStringList cornellPath = { prePath + "floor.obj", prePath + "left.obj", prePath + "right.obj", prePath + "light.obj", prePath + "shortbox.obj", prePath + "tallbox.obj" };
	Model* cornellSceneModel = new Model(cornellPath);
	for (auto& item : cornellSceneModel->getMeshes()) objects.push_back(item);
	for (auto& item : input_model->getMeshes()) input_faces.push_back(item->app_ani_faces);
	Vector3D moveVec = cornellSceneModel->modelCenter - input_model->modelCenter;
	double scaleNum = cornellSceneModel->getYRange() / input_model->getYRange() * 0.5;
	for (auto& mesh : input_faces) {
		for (auto& tri : mesh) {
			tri.v0.worldPos *= scaleNum; tri.v1.worldPos *= scaleNum; tri.v2.worldPos *= scaleNum;
			tri.v0.worldPos += moveVec; tri.v1.worldPos += moveVec; tri.v1.worldPos += moveVec;
		}
	}
    backgroundColor = bkColor;
    camera.setFov(40.f);
}

Intersection CornellBoxScene::intersect(const Ray& ray) const
{
	return this->bvh->Intersect(ray);
}

void CornellBoxScene::buildBVH() {
	printf(" - Generating BVH...\n\n");
	this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Vector3D CornellBoxScene::castRay(const Ray& ray, int depth) const
{
    Vector3D hitColor = this->backgroundColor;
    Intersection shader_point_inter = CornellBoxScene::intersect(ray);
    if (shader_point_inter.happened) {

        Vector3D p = shader_point_inter.coords;
        Vector3D N = shader_point_inter.normal;
        Vector3D wo = ray.direction;
        Vector3D L_dir(0), L_indir(0);

        Intersection light_point_inter;
        float pdf_light;
        sampleLight(light_point_inter, pdf_light);
        Vector3D x = light_point_inter.coords;
        Vector3D NN = light_point_inter.normal;
        Vector3D ws = normalize(x - p);
        Vector3D emit_I = light_point_inter.emit_I;
        float distance_pTox = glm::length(x - p);
        Vector3D p_deviation = glm::dot(ray.direction, N) < 0 ? p + N * EPSILON : p - N * EPSILON;

        Ray ray_pTox(p_deviation, ws);

        Intersection block_point_inter = CornellBoxScene::intersect(ray_pTox);
        if (abs(distance_pTox - block_point_inter.distance < 0.01)) {
            L_dir = emit_I * shader_point_inter.m->eval(wo, ws, N) * glm::dot(ws, N) * glm::dot(-ws, NN) / (distance_pTox * distance_pTox * pdf_light);
        }
        float ksi = get_random_float();
        if (ksi < RussianRoulette) {
            Vector3D wi = normalize(shader_point_inter.m->sample(wo, N));
            Ray ray_pTowi(p_deviation, wi);
            Intersection bounce_point_inter = CornellBoxScene::intersect(ray_pTowi);

            if (bounce_point_inter.happened && !bounce_point_inter.m->hasEmission()) {
                float pdf = shader_point_inter.m->pdf(wo, wi, N);
                if (pdf > EPSILON)
                    L_indir = castRay(ray_pTowi, depth + 1) * shader_point_inter.m->eval(wo, wi, N) * glm::dot(wi, N) / (pdf * RussianRoulette);
            }
        }
        hitColor = shader_point_inter.m->m_emission + L_dir + L_indir;
        hitColor.x = (clamp(0, 1, hitColor.x));
        hitColor.y = (clamp(0, 1, hitColor.y));
        hitColor.z = (clamp(0, 1, hitColor.z));
    }
    return hitColor;
}

void CornellBoxScene::sampleLight(Intersection& pos, float& pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum) {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool CornellBoxScene::trace(const Ray& ray, const std::vector<BVHItem*>& objects, float& tNear, uint32_t& index, BVHItem** hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2D uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}
// Implementation of Path Tracing

void CornellBoxScene::cornellBoxRender() {
    float scale = tan((camera.fov * 0.5) * M_PI / 180.0);
    float imageAspectRatio = width_cornellBox / (float)height_cornellBox;
    camera.setPositon(Vector3D(278, 273, -800));
    int m = 0;

    // change the spp value to change sample ammount
    int spp = 512;
    int thread_num = 6;
    int thread_height = height_cornellBox / thread_num;
    std::vector<std::thread> threads(thread_num);
    std::cout << "SPP: " << spp << "\n";

    std::mutex mtx;
    float process = 0;
    float Reciprocal_Scene_height = 1.f / (float)height_cornellBox;

    // muti-thread

    //auto castRay = [&](int thread_index) {
    //    int height = thread_height * (thread_index + 1);
    //    for (uint32_t j = height - thread_height; j < height; j++) {
    //        for (uint32_t i = 0; i < scene.width; ++i) {
    //            // generate primary ray direction
    //            float x = (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale;
    //            float y = (1 - 2 * (j + 0.5) / (float)height_cornellBox) * scale;
    //            Vector3f dir = normalize(Vector3f(-x, y, 1));
    //            for (int k = 0; k < spp; k++) {
    //                framebuffer[j * scene.width + i] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
    //            }
    //        }
    //        mtx.lock();
    //        process = process + Reciprocal_Scene_height;
    //        UpdateProgress(process);
    //        mtx.unlock();
    //    }
    //};
    //for (int k = 0; k < thread_num; k++) {
    //    threads[k] = std::thread(castRay, k);
    //}
    //for (int k = 0; k < thread_num; k++) {
    //    threads[k].join();
    //}

    // no muti-thread

    for (uint32_t j = 0; j < height_cornellBox; ++j) {
        for (uint32_t i = 0; i < width_cornellBox; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5) / (float)width_cornellBox - 1) * imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)height_cornellBox) * scale;
            Vector3D dir = normalize(Vector3D(-x, y, 1));
            Vector3D framebuffer(0.0, 0.0, 0.0);
            for (int k = 0; k < spp; k++) {
                Vector3D color = castRay(Ray(camera.position, dir), 0);
                color.x /= spp; color.y /= spp; color.z /= spp;
                framebuffer +=  color;
            }
            frame.setPixel(i, j, framebuffer);
            m++;
        }
        UpdateProgress(j / (float)height_cornellBox);
    }

    UpdateProgress(1.f);
}