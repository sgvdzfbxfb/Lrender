#ifndef MESH_H
#define MESH_H

#include <QString>
#include "BasicDataStructure.hpp"
#include "Texture.h"
#include "SRendererDevice.h"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<std::vector<unsigned>> faces;
    std::map<int, std::vector<int>> verToFace;
    int diffuseTextureIndex{-1};
    int specularTextureIndex{-1};
    Mesh() = default;
    void Draw();
};

#endif // MESH_H
