#pragma once

#include "base_object.h"

class File : public BaseObject {
private:
    std::string content;

public:
    void setContent(std::string content);
    std::string getContent();
    
    File(std::streampos start, std::string name, std::string content);
    
    void serialize(std::fstream& fileStream) override;
    static File* deserialize(std::fstream& fileStream);
    
    void print(std::ostream& out = std::cout, int nestedDegree = 0) override;
};