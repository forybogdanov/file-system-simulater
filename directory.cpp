#include "directory.h"

BaseObject* Directory::getChild(std::string name) {
    if (children.find(name) == children.end()) {
        throw std::invalid_argument("Directory not found");
    }
    return children[name];
}

Directory::Directory(std::streampos start, std::string name) : BaseObject(start, DIRTYPE, name) {}
void Directory::addChild(BaseObject* child) {
    if (children.find(child->getName()) != children.end()) {
        throw std::invalid_argument("Child already exists");
    }
    children[child->getName()] = child;
}

void Directory::deleteChild(std::string name) {
    if (children.find(name) == children.end()) {
        throw std::invalid_argument("Child not found");
    }

    children.erase(name);
}

size_t Directory::getChildrenCount() {
    return children.size();
}

void Directory::serialize(std::fstream& fileStream) {


    if (!fileStream.is_open()) {
        std::ios_base::failure("File not found");
    }

    BaseObject::serialize(fileStream);

    std::size_t childrenCount = children.size();

    fileStream.write(reinterpret_cast<char const *>(&childrenCount), sizeof(std::size_t));

    for(auto child : children) {
        if (child.second->getType() == DIRTYPE) {
            Directory* dir = dynamic_cast<Directory*>(child.second);
            dir->serialize(fileStream);
        } else {
            File* file = dynamic_cast<File*>(child.second);
            file->serialize(fileStream);
        }
    }
}

Directory* Directory::deserializeFull(std::fstream& fileStream) {

    if (!fileStream.is_open()) {
        std::ios_base::failure("File not found");
    }

    std::streampos start = fileStream.tellg();

    std::string name = readString(fileStream);
    Directory* dir = new Directory(start, name);

    std::size_t childrenCount;
    fileStream.read(reinterpret_cast<char*>(&childrenCount), sizeof(std::size_t));

    for (int i = 0; i < childrenCount; i++) {
        FileSystemObjectType type;
        fileStream.read(reinterpret_cast<char*>(&type), sizeof(FileSystemObjectType));
        if (type == DIRTYPE) {
            Directory* dir2 = Directory::deserializeFull(fileStream);
            dir->addChild(dir2);
        } else {
            File* file = File::deserialize(fileStream);
            dir->addChild(file);
        }
    }

    return dir;
}

Directory* Directory::deserializeShallow(std::fstream& fileStream) {

    if (!fileStream.is_open()) {
        std::ios_base::failure("File not found");
    }

    std::streampos start = fileStream.tellg();

    std::string name = readString(fileStream);
    Directory* dir = new Directory(start, name);

    std::size_t childrenCount;
    fileStream.read(reinterpret_cast<char*>(&childrenCount), sizeof(std::size_t));

    try {

        for (int i = 0; i < childrenCount; i++) {
            FileSystemObjectType type;
            fileStream.read(reinterpret_cast<char*>(&type), sizeof(FileSystemObjectType));
            if (type == DIRTYPE) {
                Directory* dir2 = Directory::deserializeShallow(fileStream);
                dir2->children.clear();
                dir->addChild(dir2);
            } else {
                File* file = File::deserialize(fileStream);
                dir->addChild(file);
            }
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return dir;
}

Directory* Directory::deserializeFullFrom(std::fstream& fileStream, std::streampos start) {

    if (!fileStream.is_open()) {
        std::ios_base::failure("File not found");
    }

    fileStream.seekg(start);

    return deserializeFull(fileStream);
}

Directory* Directory::deserializeShallowFrom(std::fstream& fileStream, std::streampos start) {

    if (!fileStream.is_open()) {
        std::ios_base::failure("File not found");
    }

    fileStream.seekg(start);

    return Directory::deserializeShallow(fileStream);
}

void Directory::print(std::ostream& out, int nestedDegree) {
    printNestness(out, nestedDegree);
    out << "(Directory)" << name << std::endl;
    for(auto child : children) {
        child.second->print(out, nestedDegree+1);
    }
}
