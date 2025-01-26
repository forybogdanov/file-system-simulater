#include "Utils.h"

std::string readString(std::fstream& fileStream) {
    std::size_t length;
    fileStream.read(reinterpret_cast<char*>(&length), sizeof(std::size_t));
    char* str = new char[length + 1];
    fileStream.read(str, length);
    str[length] = '\0';
    std::string res = std::string(str);
    delete[] str;
    return res;
}

std::vector<std::string> splitString(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::string current = "";
    for (char c : str) {
        if (c == delimiter) {
            result.push_back(current);
            current = "";
        }
        else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

size_t findPosition(std::string str, std::string toFind, size_t start) {
    for (size_t i = start; i < str.size(); i++) {
        if (str.substr(i, toFind.size()) == toFind) {
            return i;
        }
    }
    return std::string::npos;
}

bool includesSome(std::string str, std::vector<char> chars) {
    for (char c : chars) {
        if (str.find(c) != std::string::npos) {
            return true;
        }
    }
    return false;
}