#ifndef MODEL_H
#define MODEL_H
#include <cfloat>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "BasicDataStructure.hpp"
#include "Mesh.h"

class Model
{
    public:
        Model(QString path);
        void Draw();
        Coord3D centre;
        int triangleCount{0};
        int vertexCount{0};
        float GetYRange(){return maxY - minY;}
        bool loadSuccess{ true };
    private:
        float minX{FLT_MAX};
        float minY{FLT_MAX};
        float minZ{FLT_MAX};
        float maxX{FLT_MIN};
        float maxY{FLT_MIN};
        float maxZ{FLT_MIN};
        std::vector<Mesh> meshes;
        std::vector<Texture> textureList;
        QString directory;
        void loadModel(QString path);
        void computeNormal(int meshIdx);
        int loadMaterialTextures(QString path, std::string type);
};

#endif // MODEL_H
