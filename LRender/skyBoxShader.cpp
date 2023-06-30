#include "skyBoxShader.h"

void SkyBoxShader::vertexShader(Vertex &vertex)
{
    vertex.worldPos = Coord3D(modelMat * Coord4D(vertex.worldPos, 1.f));
    vertex.clipPos = projectionMat * viewMat * Coord4D(vertex.worldPos, 1.f);
}

int getCubemapFaceIdx(Vector3D direction, Coord2D texcoord) {
    float abs_x = (float)fabs(direction.x);
    float abs_y = (float)fabs(direction.y);
    float abs_z = (float)fabs(direction.z);
    float ma, sc, tc;
    int face_index;

    if (abs_x > abs_y && abs_x > abs_z) {   /* major axis -> x */
        ma = abs_x;
        if (direction.x > 0) {                  /* positive x */
            face_index = 0;
            sc = -direction.z;
            tc = -direction.y;
        }
        else {                                /* negative x */
            face_index = 1;
            sc = +direction.z;
            tc = -direction.y;
        }
    }
    else if (abs_y > abs_z) {             /* major axis -> y */
        ma = abs_y;
        if (direction.y > 0) {                  /* positive y */
            face_index = 2;
            sc = +direction.x;
            tc = +direction.z;
        }
        else {                                /* negative y */
            face_index = 3;
            sc = +direction.x;
            tc = -direction.z;
        }
    }
    else {                                /* major axis -> z */
        ma = abs_z;
        if (direction.z > 0) {                  /* positive z */
            face_index = 4;
            sc = +direction.x;
            tc = -direction.y;
        }
        else {                                /* negative z */
            face_index = 5;
            sc = -direction.x;
            tc = -direction.y;
        }
    }

    texcoord.x = (sc / ma + 1) / 2;
    texcoord.y = (tc / ma + 1) / 2;
    return face_index;
}

void SkyBoxShader::fragmentShader(Fragment& fragment)
{
    Color diffuseColor = {0.0f, 0.0f, 0.0f};
    Color specularColor = {0.0f, 0.0f, 0.0f};
    Vector3D viewDir = glm::normalize(eyePos - Coord3D(fragment.screenPos, 1.0));
    int cfId = getCubemapFaceIdx(viewDir, fragment.texUv);
    diffuseColor = renderAPI::API().skyBox[cfId].getColorFromUv(fragment.texUv);
    Color result(0.f, 0.f, 0.f);
    result = diffuseColor;
    fragment.fragmentColor = result;
}
