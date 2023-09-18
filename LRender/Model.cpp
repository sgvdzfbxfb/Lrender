#include "model.h"
#include <QDebug>

Model::Model(QStringList paths)
{
    QList<QString>::Iterator path = paths.begin(), itend = paths.end();
    for (int i = 0; path != itend; path++, i++) {
        std::vector<std::string> folderSp = splitString((*(path)).toStdString(), "/");
        if (i == 0) {
            for (int k = 0; k < folderSp.size() - 1; ++k) folderPath += folderSp.at(k) + "/";
            folderPath.pop_back();
        }
        std::string meshName = folderSp.at(folderSp.size() - 1).substr(0, folderSp.at(folderSp.size() - 1).length() - 4);
        meshNames.push_back(meshName);
    }

    loadModel(paths);
    modelCenter = {(maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f};

    ifModelAnimation = skeleton.skeleton_load(folderPath);
    if (ifModelAnimation) fTimeCounter.start();
}

void Model::modelRender()
{
    if (ifModelAnimation) updateModelSkeleton((float)fTimeCounter.elapsed() / 1000.0);
    for(int i = 0; i < meshes.size(); i++) meshes.at(i)->meshRender();
}

void Model::updateModelSkeleton(float ft)
{
    Skeleton skTemp = skeleton;
    if (skTemp.ske.joints.size() != 0) {
        skTemp.skeleton_update_joints(&(skTemp.ske), ft);
        renderAPI::API().shader->joint_matrices = skTemp.ske.joint_matrices;
        renderAPI::API().shader->joint_n_matrices = skTemp.ske.normal_matrices;
    }
}

void Model::loadModel(QStringList paths)
{
    QList<QString>::Iterator path = paths.begin(), itend = paths.end();
    std::vector<std::string> texPaths;
    getAllTypeFiles(folderPath, texPaths, "png");
    for (int i = 0; path != itend; path++, i++) {
        sigMesh* tempMesh = new sigMesh(*path, texPaths, meshNames.at(i));
        minX = std::min(minX, tempMesh->minX_sig); minY = std::min(minY, tempMesh->minY_sig); minZ = std::min(minZ, tempMesh->minZ_sig);
        maxX = std::max(maxX, tempMesh->maxX_sig); maxY = std::max(maxY, tempMesh->maxY_sig); maxZ = std::max(maxZ, tempMesh->maxZ_sig);
        if (!tempMesh->ifAnimation) ifModelAnimation = false;
        vertexNum += tempMesh->vertices.size();
        faceNum += tempMesh->faces.size();
        meshes.push_back(tempMesh);
    }
}
