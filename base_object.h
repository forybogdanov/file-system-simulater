#pragma once

#include "utils.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

enum FileSystemObjectType {
    DIRTYPE = 0,
    FILETYPE = 1,
};

class BaseObject {
protected:
    std::string name;
    FileSystemObjectType type;
    std::streampos start;
    void printNestness(std::ostream& out = std::cout, int nestedDegree = 0);

public:
    FileSystemObjectType getType();
    void setName(std::string name);
    std::string getName();
    std::streampos getStart();
    
    BaseObject(std::streampos start, FileSystemObjectType t, std::string name);
    
    virtual void serialize(std::fstream& file);
    virtual void print(std::ostream& out = std::cout, int nestedDegree = 0) = 0;
};
