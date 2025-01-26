#pragma once

#include "BaseObject.h"

class File : public BaseObject {
private:
    std::string content;
public:
    File(std::streampos start, std::string name, std::string content);
    void setContent(std::string content);
    std::string getContent();
    void serialize(std::fstream& fileStream);
    static File* deserialize(std::fstream& fileStream);
    void print(std::ostream& out = std::cout, int nestedDegree = 0) const;
};
