#ifndef MODEL_H
#define MODEL_H
#include <cfloat>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "dataType.h"
#include "sigMesh.h"

class Model
{
    public:
        Model(QStringList paths);
        void draw();
        Coord3D modelCenter;
        int faceNum{0};
        int vertexNum{0};
        float getYRange(){return maxY - minY;}
        bool loadSuccess{ true };
    private:
        float minX{FLT_MAX};
        float minY{FLT_MAX};
        float minZ{FLT_MAX};
        float maxX{FLT_MIN};
        float maxY{FLT_MIN};
        float maxZ{FLT_MIN};
        std::vector<sigMesh> meshes;
        std::vector<Texture> textureList;
        QString directory;
        void loadModel(QStringList paths);
        void computeNormal(sigMesh& inMesh);
        int loadMaterialTextures(QString path, std::string type);
};

#endif // MODEL_H
