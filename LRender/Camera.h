#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include "lrenderBasicCore.h"
#include "corecrt_math_defines.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum cameraPara {FOV,NEAR};
//Orbit Camera
class Camera
{
public:
    Camera(float _aspect, float far):aspect(_aspect),zFar(far){}
    float aspect = 4.f / 3.f;
    Vector3D position = Vector3D(0.0, 0.0, 0.0);
    Vector3D target = Vector3D(0.0, 0.0, 0.0);
    float zNear = 1.0f;
    float zFar = 100.f;
    float fov = 60.f;
    void rotateAroundTarget(Vector2D motion);
    void moveTarget(Vector2D motion);
    void closeToTarget(int ratio);
    void setModel(Coord3D modelCenter, float yRange);
    void setPositon(Vector3D new_p);
    Vector3D getPositon();
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
};

#endif // CAMERA_H
