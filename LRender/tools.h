#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <bitset>
#include <qDebug>
#include "dataType.h"

using namespace std::filesystem;

std::vector<std::string> splitString(const std::string& str, const std::string& delim);

bool checkIsDir(const std::string& dir);

void getAllImageFiles(const std::string dir, std::vector<std::string>& files);