#include "file_system_manager.h"

FileSystemManager::FileSystemManager(std::string path) {
    std::fstream fileStream(path);
    filename = path;

    if(!fileStream.is_open()) {
        std::invalid_argument("File not found");
    }

    FileSystemObjectType type;
    fileStream.read(reinterpret_cast<char*>(&type), sizeof(FileSystemObjectType));
    
    if (type == FILETYPE) {
        std::invalid_argument("Root cannot be a file");
    }

    Directory* current = Directory::deserializeShallow(fileStream);

    history.push(current);

    fileStream.close();
}

void FileSystemManager::goIn(std::string name) {
    Directory* current = history.top();
    BaseObject* child = current->getChild(name);

    if (child->getType() != DIRTYPE) {
        throw std::invalid_argument("Cannot go into a file");
    }
    
    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    Directory* nextDir = Directory::deserializeShallowFrom(fileStream, child->getStart());
    history.push(nextDir);

    fileStream.close();
}

void FileSystemManager::goBack() {
    if (history.size() > 1) {
        history.pop();
    }
}

void FileSystemManager::manageMoveCommand(std::string command) {
    if (command == ".") {
        return;
    } else if (command == "..") {
        goBack();
    } else {
        goIn(command);
    }
}

void FileSystemManager::overwriteFile(Directory* current) {
    std::fstream fileStream(filename, std::ios::in | std::ios::out | std::ios::binary);
    
    fileStream.seekg(0, std::ios::end);
    std::streampos fileSize = fileStream.tellg();
    
    fileStream.seekg(current->getStart());
    Directory* toMovePointer = Directory::deserializeFull(fileStream);
    delete toMovePointer;
    
    std::streampos afterCurrent = fileStream.tellg();
    std::streamsize dataSize = fileSize - afterCurrent;
    
    std::vector<char> buffer(dataSize);
    fileStream.read(buffer.data(), dataSize);
    
    std::streampos start = current->getStart();
    start -= sizeof(FileSystemObjectType);
    fileStream.seekp(start);
    
    current->serialize(fileStream);
    fileStream.write(buffer.data(), dataSize);
    
    fileStream.close();
}

void FileSystemManager::createDirectory(std::string name) {
    Directory* current = history.top();

    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    Directory* currentFull = Directory::deserializeFullFrom(fileStream, current->getStart());
    fileStream.close();

    Directory* newDir = new Directory(0, name);
    currentFull->addChild(newDir);
    current->addChild(newDir);

    overwriteFile(currentFull);
}

void FileSystemManager::deleteDirectory(std::string name) {
    BaseObject* toDelete = history.top()->getChild(name);

    if (toDelete->getType() != DIRTYPE) {
        throw std::invalid_argument("Cannot delete a file");
    }

    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    fileStream.seekg(toDelete->getStart());
    Directory* toDeleteShallow = Directory::deserializeShallow(fileStream);
    fileStream.close();

    if (toDeleteShallow->getChildrenCount() > 0) {
        throw std::invalid_argument("Directory is not empty");
    }

    history.top()->deleteChild(name);
    overwriteFile(history.top());
}

void FileSystemManager::deleteFile(std::string name) {
    BaseObject* toDelete = history.top()->getChild(name);

    if (toDelete->getType() != FILETYPE) {
        throw std::invalid_argument("Cannot delete a file");
    }

    history.top()->deleteChild(name);
    overwriteFile(history.top());
}

void FileSystemManager::copyFile(std::string filename, std::string destinationPath) {
    BaseObject* toCopy = history.top()->getChild(filename);

    if (toCopy->getType() != FILETYPE) {
        throw std::invalid_argument("Cannot copy a directory");
    }

    FileSystemManager managerCopy(*this);
    std::vector<std::string> steps = splitString(destinationPath, '/');

    for (std::string step : steps) {
        managerCopy.manageMoveCommand(step);
    }

    std::fstream fileStream(managerCopy.filename, std::ios::in | std::ios::binary);
    Directory* fullDir = Directory::deserializeFullFrom(fileStream, managerCopy.history.top()->getStart());
    fileStream.close();

    File* newFile = new File(*dynamic_cast<File*>(toCopy));
    fullDir->addChild(newFile);

    overwriteFile(fullDir);
}

void FileSystemManager::printFileContent(std::string name, std::ostream& out) {
    BaseObject* file = history.top()->getChild(name);
    if (file->getType() == DIRTYPE) {
        throw std::invalid_argument("Cannot print content of a directory");
    }
    File* file2 = dynamic_cast<File*>(file);
    out << file2->getContent() << std::endl;
}

void FileSystemManager::serialize(std::fstream& fileStream) {
    history.top()->serialize(fileStream);
}

void FileSystemManager::printCurrentDirectory() {
    history.top()->print();
}
