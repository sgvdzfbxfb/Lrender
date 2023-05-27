#ifndef MESH_H
#define MESH_H

#include <QString>
#include "src/BasicDataStructure.hpp"
#include "src/Texture.h"
#include "src/SRendererDevice.h"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    int diffuseTextureIndex{-1};
    int specularTextureIndex{-1};
    Mesh() = default;
    void Draw();
};

#endif // MESH_H
