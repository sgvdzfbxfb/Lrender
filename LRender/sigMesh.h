#ifndef MESH_H
#define MESH_H

#include <QString>
#include "dataType.h"
#include "texture.h"
#include "renderAPI.h"

struct sigMesh
{
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
    sigMesh() = default;
    void meshRender();
};

#endif // MESH_H
