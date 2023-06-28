#ifndef MESH_H
#define MESH_H

#include <QString>
#include "dataType.h"
#include "texture.h"
#include "renderAPI.h"

struct sigMesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<std::vector<unsigned>> faces;
    std::map<int, std::vector<int>> verToFace;
    std::vector<Vector3D> vertNormals;
    std::vector<Coord2D> vertUVs;
    std::vector<int> diffuseIds;
    std::vector<int> specularIds;
    sigMesh() = default;
    void meshRender();
};

#endif // MESH_H
