#include "utils.h"

std::string readString(std::fstream& fileStream) {
    std::size_t length;
    fileStream.read(reinterpret_cast<char*>(&length), sizeof(std::size_t));
    char* str = new char[length+1];
    fileStream.read(str, length);
    str[length] = '\0';
    return std::string(str);
}

std::vector<std::string> splitString(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::string current = "";
    for (char c : str) {
        if (c == delimiter) {
            result.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}