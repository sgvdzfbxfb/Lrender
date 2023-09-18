#ifndef MODEL_H
#define MODEL_H
#include <cfloat>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <QTime>
#include <filesystem>
#include <stdlib.h>
#include "sigMesh.h"
#include "tools.h"
#include "skeleton.h"

class Model
{
    public:
        Model(QStringList paths);
        void modelRender();
        Coord3D modelCenter;
        int faceNum{0};
        int vertexNum{0};
        float getYRange() { return maxY - minY; }
        bool loadSuccess{ true };
        Skeleton skeleton;
        QTime fTimeCounter;
        std::vector<sigMesh*> getMeshes() { return meshes; };
    private:
        float minX{FLT_MAX};
        float minY{FLT_MAX};
        float minZ{FLT_MAX};
        float maxX{FLT_MIN};
        float maxY{FLT_MIN};
        float maxZ{FLT_MIN};
        std::vector<sigMesh*> meshes;
        std::vector<Texture> textureList;
        bool ifModelAnimation = false;
        QString directory;
        std::string folderPath;
        std::vector<std::string> meshNames;
        void loadModel(QStringList paths);
        void updateModelSkeleton(float ft);
};

#endif // MODEL_H
