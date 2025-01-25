#pragma once

#include "file.h"
#include <unordered_map>

class Directory : public BaseObject {
private:
    std::unordered_map<std::string, BaseObject*> children;

public:
    BaseObject* getChild(std::string name);
    Directory(std::streampos start, std::string name);
    void addChild(BaseObject* child);
    void deleteChild(std::string name);
    size_t getChildrenCount();
    
    void serialize(std::fstream& fileStream) override;

    static Directory* deserializeFullFrom(std::fstream& fileStream, std::streampos start);
    static Directory* deserializeShallowFrom(std::fstream& fileStream, std::streampos start);
    static Directory* deserializeFull(std::fstream& fileStream);
    static Directory* deserializeShallow(std::fstream& fileStream);
    
    void print(std::ostream& out = std::cout, int nestedDegree = 0) override;
};
