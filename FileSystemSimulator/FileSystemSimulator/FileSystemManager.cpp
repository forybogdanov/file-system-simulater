#include "FileSystemManager.h"
#include "Utils.h"

void FileSystemManager::refreshCurrentDirectory() {
    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    Directory* newCurrent = Directory::deserializeShallowFrom(fileStream, history.top()->getStart());
    history.pop();
    history.push(newCurrent);
    fileStream.close();
}

void FileSystemManager::manageMoveCommand(std::string command) {
    if (command == ".") {
        return;
    }
    else if (command == "..") {
        goBack();
    }
    else {
        goIn(command);
    }
}

FileSystemManager::FileSystemManager(std::string path) {
    std::fstream fileStream(path, std::ios::in | std::ios::binary);
    filename = path;

    if (!fileStream.is_open()) {
        std::fstream fileStream2(path, std::ios::out | std::ios::binary);
        Directory* root = new Directory(0, "root");
        root->serialize(fileStream2);
        fileStream2.close();
        delete root;

        fileStream.open(path, std::ios::in | std::ios::binary);
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

    if (child == nullptr) {
        throw std::invalid_argument("Dir not found");
    }

    if (child->getType() != DIRTYPE) {
        throw std::invalid_argument("Cannot go into a file");
    }

    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    fileStream.seekg(child->getStart());

    Directory* newCurrent = Directory::deserializeShallow(fileStream);
    history.push(newCurrent);

    fileStream.close();
}

void FileSystemManager::goBack() {
    if (history.size() > 1) {
        history.pop();
    }

    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    Directory* newCurrent = Directory::deserializeShallowFrom(fileStream, history.top()->getStart());
    history.pop();
    history.push(newCurrent);
}

void FileSystemManager::manageMoveCommandMultiple(std::string command) {
    if (command[0] == '/') {
        history = std::stack<Directory*>();
        std::fstream fileStream(filename, std::ios::in | std::ios::binary);
        FileSystemObjectType type;
        fileStream.read(reinterpret_cast<char*>(&type), sizeof(FileSystemObjectType));
        Directory* current = Directory::deserializeShallow(fileStream);
        fileStream.close();
        history.push(current);
        command = command.substr(6); // remove the first / and the root and the after that /
    }

    try {
        std::vector<std::string> steps = splitString(command, '/');
        for (std::string step : steps) {
            manageMoveCommand(step);
        }
    }
    catch (...) {
        throw std::invalid_argument("Invalid path");
    }
}

void FileSystemManager::overwriteFile(BaseObject* current) {
    std::fstream fileStream(filename, std::ios::in | std::ios::out | std::ios::binary);
    
    fileStream.seekg(0, std::ios::end);
    std::streampos fileSize = fileStream.tellg();

    fileStream.seekg(current->getStart());

    if (current->getType() == FILETYPE) {
        File* toMovePointer = File::deserialize(fileStream);
        delete toMovePointer;
    }
    else {
        Directory* toMovePointer = Directory::deserializeFull(fileStream);
        delete toMovePointer;
    }

    std::streampos afterCurrent = fileStream.tellg();
    std::streamsize bufferSize = fileSize - afterCurrent;

    std::vector<char> buffer(bufferSize);
    fileStream.read(buffer.data(), bufferSize);

    std::streampos start = current->getStart();
    start -= sizeof(FileSystemObjectType);
    fileStream.seekp(start);

    if (current->getType() == FILETYPE) {
        File* file = dynamic_cast<File*>(current);
        file->serialize(fileStream);
    }
    else {
        Directory* dir = dynamic_cast<Directory*>(current);
        dir->serialize(fileStream);
    }

    fileStream.write(buffer.data(), bufferSize);

    fileStream.close();
}

void FileSystemManager::createDirectory(std::string name) {
    Directory* current = history.top();

    if (name.empty()) {
        throw std::invalid_argument("Name cannot be empty");
    }

    if (includesSome(name, { '/', '\\' })) {
        throw std::invalid_argument("Name cannot contain '/' or '\\'");
    }

    if (current->getChild(name) != nullptr) {
        throw std::invalid_argument("Dir already exists");
    }

    std::fstream fileStream(filename, std::ios::in | std::ios::binary);
    Directory* currentFull = Directory::deserializeFullFrom(fileStream, current->getStart());
    fileStream.close();

    Directory* newDir = new Directory(0, name);
    currentFull->addChild(newDir);
    overwriteFile(currentFull);
    delete currentFull;

    fileStream.open(filename, std::ios::in | std::ios::binary);
    Directory* newCurrent = Directory::deserializeShallowFrom(fileStream, current->getStart());
    history.pop();
    history.push(newCurrent);
}

void FileSystemManager::deleteDirectory(std::string name) {
    BaseObject* toDelete = history.top()->getChild(name);

    if (toDelete == nullptr) {
        throw std::invalid_argument("Child not found");
    }

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

    std::fstream fileStream2(filename, std::ios::in | std::ios::binary);
    Directory* fullDir = Directory::deserializeFullFrom(fileStream2, history.top()->getStart());
    fileStream2.close();

    fullDir->deleteChild(name);

    overwriteFile(fullDir);
    delete fullDir;
}

void FileSystemManager::deleteFile(std::string name) {
    BaseObject* toDelete = history.top()->getChild(name);

    if (toDelete == nullptr) {
        throw std::invalid_argument("File not found");
    }

    if (toDelete->getType() != FILETYPE) {
        throw std::invalid_argument("Cannot delete a file");
    }

    history.top()->deleteChild(name);

    std::fstream fileStream2(filename, std::ios::in | std::ios::binary);
    Directory* fullDir = Directory::deserializeFullFrom(fileStream2, history.top()->getStart());
    fileStream2.close();

    fullDir->deleteChild(name);

    overwriteFile(fullDir);
    delete fullDir;
}

void FileSystemManager::copyFile(std::string path, std::string destinationPath) {
    std::string filename = path.substr(path.find_last_of("/\\") + 1);

    path = path.substr(0, path.find_last_of("/\\"));

    FileSystemManager managerCopy(*this);
    managerCopy.manageMoveCommandMultiple(path);

    BaseObject* toCopy = managerCopy.history.top()->getChild(filename);

    if (toCopy == nullptr) {
        throw std::invalid_argument("Child not found");
    }

    if (toCopy->getType() != FILETYPE) {
        throw std::invalid_argument("Cannot copy a directory");
    }

    if (destinationPath == path) {
        throw std::invalid_argument("Cannot copy to the same directory");
    }

    managerCopy = FileSystemManager(*this);

    managerCopy.manageMoveCommandMultiple(destinationPath);

    std::fstream fileStream(managerCopy.filename, std::ios::in | std::ios::binary);
    Directory* fullDir = Directory::deserializeFullFrom(fileStream, managerCopy.history.top()->getStart());
    fileStream.close();

    if (fullDir->getChild(toCopy->getName()) != nullptr) {
        fullDir->deleteChild(toCopy->getName());
    }

    File* newFile = new File(*dynamic_cast<File*>(toCopy));
    fullDir->addChild(newFile);

    overwriteFile(fullDir);
    delete fullDir;

    refreshCurrentDirectory();
}

void FileSystemManager::writeToFile(std::string name, std::string content) {
    BaseObject* file = history.top()->getChild(name);

    if (includesSome(name, { '/', '\\' })) {
        throw std::invalid_argument("File name cannot contain '/' or '\\'");
    }

    if (content.size() < 3) {
        content = "";
    }
    else {
        content = content.substr(2).substr(0, content.length() - 3);
    }

    if (file == nullptr) {
        File* newFile = new File(0, name, content);
        history.top()->addChild(newFile);

        std::fstream fileStream2(filename, std::ios::in | std::ios::binary);
        Directory* fullDir = Directory::deserializeFullFrom(fileStream2, history.top()->getStart());
        fileStream2.close();

        fullDir->addChild(newFile);

        overwriteFile(fullDir);
        delete fullDir;

        refreshCurrentDirectory();

        return;
    }

    if (file->getType() == DIRTYPE) {
        throw std::invalid_argument("Cannot write to a directory");
    }

    File* file2 = dynamic_cast<File*>(file);
    std::string toWrite = "";

    if (content.size() == 0) {
        toWrite = "";
    }
    else {
        toWrite = file2->getContent() + content;
    }

    file2->setContent(toWrite);
    overwriteFile(file2);
    refreshCurrentDirectory();
}

void FileSystemManager::importFile(std::string source, std::string destination, std::string append) {
    std::fstream fileStream(source, std::ios::in | std::ios::binary);
    std::string filename;

    if (!fileStream.is_open()) {
        throw std::invalid_argument("File not found");
    }

    if (source.find_last_of("/\\") != std::string::npos) {
        filename = source.substr(source.find_last_of("/\\"));
    }
    else {
        filename = source;
    }

    if (append.size() < 3) append = "";
    else append = append.substr(2).substr(0, append.length() - 3);

    std::string content = "";
    std::string line;
    while (std::getline(fileStream, line)) {
        content += line + "\n";
    }
    fileStream.close();

    content += append;

    if (findPosition(destination, filename, 0) != std::string::npos) {
        destination = destination.substr(0, destination.find_last_of("/\\"));
    }

    FileSystemManager managerCopy(*this);
    managerCopy.manageMoveCommandMultiple(destination);

    File* newFile = new File(0, filename, content);

    if (managerCopy.history.top()->getChild(filename) != nullptr) {
        managerCopy.history.top()->deleteChild(filename);
    }

    managerCopy.history.top()->addChild(newFile);

    std::fstream fileStream2(managerCopy.filename, std::ios::in | std::ios::binary);
    Directory* fullDir = Directory::deserializeFullFrom(fileStream2, managerCopy.history.top()->getStart());
    fileStream2.close();

    if (fullDir->getChild(filename) != nullptr) {
        fullDir->deleteChild(filename);
    }
    fullDir->addChild(newFile);

    overwriteFile(fullDir);
    delete fullDir;
}

void FileSystemManager::exportFile(std::string source, std::string destination) {
    BaseObject* possibleFile = history.top()->getChild(source);

    if (possibleFile == nullptr) {
        throw std::invalid_argument("File not found");
    }

    if (possibleFile->getType() == DIRTYPE) {
        throw std::invalid_argument("Cannot export a directory");
    }

    File* file = dynamic_cast<File*>(possibleFile);
    std::fstream fileStream(destination, std::ios::out);

    if (!fileStream.is_open()) {
        throw std::runtime_error("Cannot create file");
    }

    fileStream << file->getContent();
    fileStream.close();
}
void FileSystemManager::printFileContent(std::string name, std::ostream& out) {
    BaseObject* file = history.top()->getChild(name);

    if (file == nullptr) {
        throw std::invalid_argument("File not found");
    }

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