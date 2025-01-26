#include<iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<exception>
#include<stack>
#include<vector>

enum FileSystemObjectType {
    DIRTYPE = 0,
    FILETYPE = 1,
};

std::string readString(std::fstream& fileStream) {
    std::size_t length;
    fileStream.read(reinterpret_cast<char*>(&length), sizeof(std::size_t));
    char* str = new char[length+1];
    fileStream.read(str, length);
    str[length] = '\0';
    std::string res = std::string(str);
    delete[] str;
    return res;
}

std::vector<std::string> splitString(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::string current = "";
    for (char c : str) {
        if (c == delimiter) {
            result.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

size_t findPosition(std::string str, std::string toFind, size_t start) {
    for (size_t i = start; i < str.size(); i++) {
        if (str.substr(i, toFind.size()) == toFind) {
            return i;
        }
    }
    return std::string::npos;
}

bool includesSome(std::string str, std::vector<char> chars) {
    for (char c : chars) {
        if (str.find(c) != std::string::npos) {
            return true;
        }
    }
    return false;
}

class BaseObject {
    protected:
    std::string name;
    FileSystemObjectType type;
    std::streampos start;
    void printNestness(std::ostream& out = std::cout, int nestedDegree = 0) {
        for (int i = 0; i < nestedDegree-1; i++) {
            out << "    ";
        }
        if (nestedDegree > 0) out << char(192) << "-->";
    }
    public:
    FileSystemObjectType getType() {
        return type;
    }
    void setName(std::string name) {
        if (name.empty()) {
            throw std::invalid_argument("Name cannot be empty");
        }
        this->name = name;
    }
    std::string getName() {
        return name;
    }
    std::streampos getStart() {
        return start;
    }
    BaseObject(std::streampos start, FileSystemObjectType t, std::string name) {
        this->start = start;
        this->name = name;
        this->type = t;
    }
    virtual ~BaseObject() {}
    void serialize(std::fstream& file) {
        if(!file.is_open()) {
            std::ios_base::failure("File not found");
        }

        file.write(reinterpret_cast<char const *>(&type), sizeof(FileSystemObjectType));
        std::size_t nameLength = name.length();
        file.write(reinterpret_cast<char const *>(&nameLength), sizeof(std::size_t));
        file.write(name.c_str(), nameLength);
    }
    virtual void print(std::ostream& out = std::cout, int nestedDegree = 0) = 0;
};

class File : public BaseObject {
    private:
    std::string content;
    public:
    void setContent(std::string content) {
        this->content = content;
    }
    std::string getContent() {
        return content;
    }
    File(std::streampos start, std::string name, std::string content) : BaseObject(start, FILETYPE, name) {
        this->content = content;
    }
    void serialize(std::fstream& fileStream) {

        BaseObject::serialize(fileStream);

        std::size_t contentLength = content.length();
        fileStream.write(reinterpret_cast<char const *>(&contentLength), sizeof(std::size_t));
        fileStream.write(content.c_str(), contentLength);
    }
    
    static File* deserialize(std::fstream& fileStream) {

        if (!fileStream.is_open()) {
            std::ios_base::failure("File not found");
        }

        std::streampos start = fileStream.tellg();

        std::string name = readString(fileStream);
        std::string content = readString(fileStream);

        return new File(start, name, content);
    }
    void print(std::ostream& out = std::cout, int nestedDegree = 0) override {
        printNestness(out, nestedDegree);
        out << "(File)" << name << std::endl;
    }
};

class Directory : public BaseObject {
    private:
    std::unordered_map<std::string, BaseObject*> children;
    public:
    BaseObject* getChild(std::string name) {

        if (children.find(name) == children.end()) {
            return nullptr;
        }
        return children[name];
    }
    Directory(std::streampos start, std::string name) : BaseObject(start, DIRTYPE, name) {}
    void addChild(BaseObject* child) {
        if (children.find(child->getName()) != children.end()) {
            throw std::invalid_argument("Child already exists");
        }
        children[child->getName()] = child;
    }
    void deleteChild(std::string name) {
        if (children.find(name) == children.end()) {
            throw std::invalid_argument("Child not found");
        }

        children.erase(name);
    }
    int getChildrenCount() {
        return children.size();
    }
    void serialize(std::fstream& fileStream) {


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
    static Directory* deserializeFullFrom(std::fstream& fileStream, std::streampos start) {

        if (!fileStream.is_open()) {
            std::ios_base::failure("File not found");
        }

        fileStream.seekg(start);

        return deserializeFull(fileStream);
    }
    static Directory* deserializeShallowFrom(std::fstream& fileStream, std::streampos start) {

        if (!fileStream.is_open()) {
            std::ios_base::failure("File not found");
        }

        fileStream.seekg(start);

        return Directory::deserializeShallow(fileStream);
    }
    static Directory* deserializeFull(std::fstream& fileStream) {

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
    static Directory* deserializeShallow(std::fstream& fileStream) {

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
                Directory* dir2 = Directory::deserializeShallow(fileStream);
                dir2->children.clear();
                dir->addChild(dir2);
            } else {
                File* file = File::deserialize(fileStream);
                dir->addChild(file);
            }
        }

        return dir;
    }
    void print(std::ostream& out = std::cout, int nestedDegree = 0) override {
        printNestness(out, nestedDegree);
        out << "(Directory)" << name << std::endl;
        for(auto child : children) {
            child.second->print(out, nestedDegree+1);
        }
    }
};

class FileSystemManager {
    private:
    std::stack<Directory*> history;
    std::string filename;
    void manageMoveCommand(std::string command) {
       if (command == ".") {
            return;
        } else if (command == "..") {
            goBack();
        } else {
            goIn(command);
        }
    }
    public:
    FileSystemManager(std::string path) {
        std::fstream fileStream(path, std::ios::in | std::ios::binary);
        filename = path;

        if(!fileStream.is_open()) {
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
    void goIn(std::string name) {
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
    void goBack() {
        if (history.size() > 1) {
            history.pop();
        }

        std::fstream fileStream(filename, std::ios::in | std::ios::binary);
        Directory* newCurrent = Directory::deserializeShallowFrom(fileStream, history.top()->getStart());
        history.pop();
        history.push(newCurrent);
    }
    void manageMoveCommandMultiple(std::string command) {

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
        } catch(...) {
            throw std::invalid_argument("Invalid path");
        }
    }
    void overwriteFile(BaseObject* current) {
        std::fstream fileStream(filename, std::ios::in | std::ios::out | std::ios::binary);
        
        fileStream.seekg(0, std::ios::end);
        std::streampos fileSize = fileStream.tellg();

        fileStream.seekg(current->getStart());

        if (current->getType() == FILETYPE) {
            File* toMovePointer = File::deserialize(fileStream);
            delete toMovePointer;
        } else {
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
        } else {
            Directory* dir = dynamic_cast<Directory*>(current);
            dir->serialize(fileStream);
        }
        
        fileStream.write(buffer.data(), bufferSize);
        
        fileStream.close();
    }
    void createDirectory(std::string name) {
        Directory* current = history.top();

        if (name.empty()) {
            throw std::invalid_argument("Name cannot be empty");
        }

        if (includesSome(name, {'/', '\\'})) {
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
    void deleteDirectory(std::string name) {
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
    void deleteFile(std::string name) {
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
    void copyFile(std::string filename, std::string destinationPath) {
        BaseObject* toCopy = history.top()->getChild(filename);

        if (toCopy == nullptr) {
            throw std::invalid_argument("Child not found");
        }

        if (toCopy->getType() != FILETYPE) {
            throw std::invalid_argument("Cannot copy a directory");
        }

        FileSystemManager managerCopy(*this);
        
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
    }
    void writeToFile(std::string name, std::string content) {
        BaseObject* file = history.top()->getChild(name);

        if (content.size() < 3) {
            content = "";
        } else {
            content = content.substr(2).substr(0, content.length()-3);
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
            return;
        }

        if (file->getType() == DIRTYPE) {
            throw std::invalid_argument("Cannot write to a directory");
        }

        File* file2 = dynamic_cast<File*>(file);
        std::string toWrite = "";

        if (content.size() == 0) {
            toWrite = "";
        } else {
            toWrite = file2->getContent() + content;
        }

        file2->setContent(toWrite);
        overwriteFile(file2);
    }
    void importFile(std::string source, std::string destination, std::string append) {
        std::fstream fileStream(source, std::ios::in | std::ios::binary);
        std::string filename;

        if (!fileStream.is_open()) {
            throw std::invalid_argument("File not found");
        }

        if (source.find_last_of("/\\") != std::string::npos) {
            filename = source.substr(source.find_last_of("/\\"));
        } else {
            filename = source;
        }

        if (append.size() < 3) append = "";
        else append = append.substr(2).substr(0, append.length()-3);
        
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
    void exportFile(std::string source, std::string destination) {
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
    void printFileContent(std::string name, std::ostream& out = std::cout) {
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
    void serialize(std::fstream& fileStream) {
        history.top()->serialize(fileStream);
    }
    void printCurrentDirectory() {
        history.top()->print();
    } 
};

void createExample() {
    File essay(0, "essay.txt", "This is an essay");
    Directory root(0, "root");
    Directory photos(0, "photos");
    root.addChild(&essay);
    root.addChild(&photos);
    File photo1(0, "photo1.jpg", "This is a photo");
    photos.addChild(&photo1);
    root.print();
    std::fstream fileStream("example.bin", std::ios::out | std::ios::binary);
    root.serialize(fileStream);
    fileStream.close();
}

int main() {
    std::string filename;
    std::cout << "Enter the file system file name: ";
    std::cin >> filename;
    FileSystemManager manager(filename+".bin");
    std::string command;
    do {
        std::cin >> command;
        try {
            if (command == "ls") {
                manager.printCurrentDirectory();

            } else if (command == "cd") {
                std::string name;
                std::cin >> name;
                manager.manageMoveCommandMultiple(name);

            } else if (command == "mkdir") {
                std::string name;
                std::cin >> name;
                manager.createDirectory(name);

            } else if (command == "rmdir") {
                std::string name;
                std::cin >> name;
                manager.deleteDirectory(name);

            } else if(command == "rm") {
                std::string name;
                std::cin >> name;
                manager.deleteFile(name);

            } else if (command == "cat") {
                std::string name;
                std::cin >> name;
                manager.printFileContent(name);
            
            } else if (command == "cp") {
                std::string name, destination;
                std::cin >> name >> destination;
                manager.copyFile(name, destination);

            } else if (command == "write") {
                std::string name, content;
                std::cin >> name;
                std::getline(std::cin, content);
                manager.writeToFile(name, content);

            } else if (command == "import") {
                std::string source, destination;
                std::cin >> source >> destination;
                std::string append;
                std::getline(std::cin, append);
                manager.importFile(source, destination, append);

            } else if (command == "export") {
                std::string source, destination;
                std::cin >> source >> destination;
                manager.exportFile(source, destination);
            
            } else if (command == "help") {
                std::cout << "ls - list current directory" << std::endl;
                std::cout << "cd <name> - move to directory" << std::endl;
                std::cout << "mkdir <name> - create directory" << std::endl;
                std::cout << "rmdir <name> - delete directory" << std::endl;
                std::cout << "rm <name> - delete file" << std::endl;
                std::cout << "cat <name> - print file content" << std::endl;
                std::cout << "cp <name> <destination> - copy file" << std::endl;
                std::cout << "write <name> <content> - write to file" << std::endl;
                std::cout << "import <source> <destination> <append> - import file" << std::endl;
                std::cout << "export <source> <destination> - export file" << std::endl;
                std::cout << "exit - exit the program" << std::endl;
            } else if (command == "createExample") {
                createExample();

            } else if (command == "exit") {
                break;
            } else {
                std::cout << command << " is not a recognized command" << std::endl;
            }
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    } while (true);
    return 0;
}