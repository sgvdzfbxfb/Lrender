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

// ���һ��·���Ƿ���Ŀ¼
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

// ����һ��Ŀ¼�����е�ͼ���ļ����� png ��β���ļ�
void getAllImageFiles(const std::string dir, std::vector<std::string>& files) {
    // ���ȼ��Ŀ¼�Ƿ�Ϊ�գ��Լ��Ƿ���Ŀ¼
    if (!checkIsDir(dir)) return;
    // �ݹ�������е��ļ�
    directory_iterator iters(dir);
    for (auto& iter : iters) {
        // ·����ƴ�ӣ�����֪ʶ���ǲ����أ�ֻ����������취����
        std::string file_path(dir);
        file_path += "/"; file_path += (iter.path().filename()).string();
        // �鿴�Ƿ���Ŀ¼�������Ŀ¼��ѭ���ݹ�
        if (checkIsDir(file_path)) {
            getAllImageFiles(file_path, files);
        }
        else { //����Ŀ¼�����׺�Ƿ���ͼ��
            std::string extension = (iter.path().extension()).string(); // ��ȡ�ļ��ĺ�׺��
            // ������չ�����Լ���Ҫ���ļ�����������ɸѡ, �������.gif .bmp֮���
            if (extension == ".png") {
                files.push_back(file_path);
            }
        }
    }
}