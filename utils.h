#pragma once

#include <string>
#include <fstream>
#include <vector>

std::string readString(std::fstream& fileStream);
std::vector<std::string> splitString(std::string str, char delimiter);