#ifndef MODEL_H
#define MODEL_H
#include <cfloat>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdlib.h>
#include "dataType.h"
#include "sigMesh.h"

class Model
{
    public:
        Model(QStringList paths);
        void modelRender();
        Coord3D modelCenter;
        int faceNum{0};
        int vertexNum{0};
        float getYRange(){return maxY - minY;}
        bool loadSuccess{ true };
        std::vector<Vertex> skyBoxVers;
        std::vector<Texture> getSkyBox();
        void loadSkyBox(std::string skyPath);
    private:
        float minX{FLT_MAX};
        float minY{FLT_MAX};
        float minZ{FLT_MAX};
        float maxX{FLT_MIN};
        float maxY{FLT_MIN};
        float maxZ{FLT_MIN};
        std::vector<sigMesh> meshes;
        std::vector<Texture> textureList;
        std::vector<Texture> cubeMapSkyBox;
        QString directory;
        void loadModel(QStringList paths);
        void computeNormal(sigMesh& inMesh);
        int getMeshTexture(std::string t_ps, std::string type);
};

#endif // MODEL_H
