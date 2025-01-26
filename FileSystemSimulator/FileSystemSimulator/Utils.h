#pragma once

#include <string>
#include <vector>
#include <fstream>

std::string readString(std::fstream& fileStream);
std::vector<std::string> splitString(std::string str, char delimiter);
size_t findPosition(std::string str, std::string toFind, size_t start);
bool includesSome(std::string str, std::vector<char> chars);