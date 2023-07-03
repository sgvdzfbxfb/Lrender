#include "tools.h"

std::vector<std::string> splitString(const std::string& str, const std::string& delim) {
    std::vector<std::string> res;
    if ("" == str) return res;
    char* strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char* d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char* p = strtok(strs, d);
    while (p) {
        std::string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }
    return res;
}

// 检查一个路径是否是目录
bool checkIsDir(const std::string& dir) {
    if (!exists(dir)) {
        qDebug() << QString::fromStdString(dir) << "not exists. Please check.";
        return false;
    }
    directory_entry entry(dir);
    if (entry.is_directory())
        return true;
    return false;
}

// 搜索一个目录下所有的图像文件，以 png 结尾的文件
void getAllImageFiles(const std::string dir, std::vector<std::string>& files) {
    // 首先检查目录是否为空，以及是否是目录
    if (!checkIsDir(dir)) return;
    // 递归遍历所有的文件
    directory_iterator iters(dir);
    for (auto& iter : iters) {
        // 路径的拼接，基础知识还是不过关，只能用这个笨办法来了
        std::string file_path(dir);
        file_path += "/"; file_path += (iter.path().filename()).string();
        // 查看是否是目录，如果是目录则循环递归
        if (checkIsDir(file_path)) {
            getAllImageFiles(file_path, files);
        }
        else { //不是目录则检查后缀是否是图像
            std::string extension = (iter.path().extension()).string(); // 获取文件的后缀名
            // 可以扩展成你自己想要的文件类型来进行筛选, 比如加上.gif .bmp之类的
            if (extension == ".png") {
                files.push_back(file_path);
            }
        }
    }
}