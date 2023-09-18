#include "cornellBoxScene.h"

CornellBoxScene::CornellBoxScene(Model* input_model) {
	QString prePath = "./cornellbox/";
	QStringList cornellPath = { prePath + "floor.obj", prePath + "left.obj", prePath + "right.obj", prePath + "light.obj", prePath + "shortbox.obj", prePath + "tallbox.obj" };
	Model* cornellSceneModel = new Model(cornellPath);
	for (auto& item : cornellSceneModel->getMeshes()) objects.push_back(item);
	for (auto& item : input_model->getMeshes()) input_faces.push_back(item->app_ani_faces);
}