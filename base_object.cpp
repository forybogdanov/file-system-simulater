#include "base_object.h"

void BaseObject::printNestness(std::ostream& out, int nestedDegree) {
    for (int i = 0; i < nestedDegree-1; i++) {
        out << "    ";
    }
    if (nestedDegree > 0) out << char(192) << "-->";
}

FileSystemObjectType BaseObject::getType() {
    return type;
}

void BaseObject::setName(std::string name) {
    if (name.empty()) {
        throw std::invalid_argument("Name cannot be empty");
    }
    this->name = name;
}

std::string  BaseObject::getName() {
    return name;
}

std::streampos  BaseObject::getStart() {
    return start;
}

BaseObject::BaseObject(std::streampos start, FileSystemObjectType t, std::string name) {
    this->start = start;
    this->name = name;
    this->type = t;
}

void BaseObject::serialize(std::fstream& file) {
    if(!file.is_open()) {
        std::ios_base::failure("File not found");
    }

    file.write(reinterpret_cast<char const *>(&type), sizeof(FileSystemObjectType));
    std::size_t nameLength = name.length();
    file.write(reinterpret_cast<char const *>(&nameLength), sizeof(std::size_t));
    file.write(name.c_str(), nameLength);
}