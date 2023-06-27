#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include "dataType.h"
#include "corecrt_math_defines.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum cameraPara {FOV,NEAR};
//Orbit Camera
class Camera
{
public:
    Camera(float _aspect, float far):aspect(_aspect),zFar(far){}
    float aspect;
    Vector3D position;
    Vector3D target;
    float zNear;
    float zFar;
    float fov;
    void rotateAroundTarget(Vector2D motion);
    void moveTarget(Vector2D motion);
    void closeToTarget(int ratio);
    void setModel(Coord3D modelCentre, float yRange);
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
};

#endif // CAMERA_H
