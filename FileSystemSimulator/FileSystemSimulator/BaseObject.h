#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

enum FileSystemObjectType {
    FILETYPE = 0,
    DIRTYPE = 1
};

class BaseObject {
protected:
    std::string name;
    FileSystemObjectType type;
    std::streampos start;

    void printNestness(std::ostream& out = std::cout, int nestedDegree = 0) const;

public:
    BaseObject(std::streampos start, FileSystemObjectType t, const std::string& name);
    virtual ~BaseObject() = 0;
    std::string getName() const;
    void setName(const std::string& name);
    FileSystemObjectType getType() const;
    std::streampos getStart() const;
    void serialize(std::fstream& file) const;
    virtual void print(std::ostream& out = std::cout, int nestedDegree = 0) const = 0;
};
