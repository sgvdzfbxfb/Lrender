#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <bitset>
#include <qDebug>
#include "triangle.h"

using namespace std::filesystem;

std::vector<std::string> splitString(const std::string& str, const std::string& delim);

bool checkIsDir(const std::string& dir);

void getAllTypeFiles(const std::string dir, std::vector<std::string>& files, std::string type);

Coord3D interpolate(float alpha, float beta, float gamma, const Coord3D& vert1, const Coord3D& vert2, const Coord3D& vert3, float weight);

Coord2D interpolate(float alpha, float beta, float gamma, const Coord2D& vert1, const Coord2D& vert2, const Coord2D& vert3, float weight);

std::vector<Triangle> constructTriangle(std::vector<Vertex> vertexList);

Fragment interpolationFragment(int x, int y, float z, Triangle& tri, Vector3D& barycentric);