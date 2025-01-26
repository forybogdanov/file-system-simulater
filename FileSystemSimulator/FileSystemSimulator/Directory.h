#pragma once

#include "File.h"
#include <unordered_map>

class Directory : public BaseObject {
private:
    std::unordered_map<std::string, BaseObject*> children;

public:
    Directory(std::streampos start, std::string name);
    BaseObject* getChild(std::string name);
    void addChild(BaseObject* child);
    void deleteChild(std::string name);
    std::size_t getChildrenCount();
    void serialize(std::fstream& fileStream);
    static Directory* deserializeFull(std::fstream& fileStream);
    static Directory* deserializeShallow(std::fstream& fileStream);
    static Directory* deserializeFullFrom(std::fstream& fileStream, std::streampos start);
    static Directory* deserializeShallowFrom(std::fstream& fileStream, std::streampos start);
    void print(std::ostream& out = std::cout, int nestedDegree = 0) const;
};
