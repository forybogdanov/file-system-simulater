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
    return std::string(str);
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
            throw std::invalid_argument("Directory not found");
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
    public:
    FileSystemManager(std::string path) {
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
    void goIn(std::string name) {
        Directory* current = history.top();
        BaseObject* child = current->getChild(name);

        if (child->getType() != DIRTYPE) {
            throw std::invalid_argument("Cannot go into a file");
        }

        history.push(dynamic_cast<Directory*>(child));
        
        std::fstream fileStream(filename, std::ios::in | std::ios::binary);
        fileStream.seekg(child->getStart());

        history.top() = Directory::deserializeShallow(fileStream);

        fileStream.close();
    }
    void goBack() {
        if (history.size() > 1) {
            history.pop();
        }
    }
    void overwriteFile(Directory* current) {
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
    void createDirectory(std::string name) {
        Directory* current = history.top();

        std::fstream fileStream(filename, std::ios::in | std::ios::binary);
        Directory* currentFull = Directory::deserializeFullFrom(fileStream, current->getStart());
        fileStream.close();

        Directory* newDir = new Directory(0, name);
        currentFull->addChild(newDir);
        current->addChild(newDir);

        overwriteFile(currentFull);
    }
    void deleteDirectory(std::string name) {
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
    void deleteFile(std::string name) {
        BaseObject* toDelete = history.top()->getChild(name);

        if (toDelete->getType() != FILETYPE) {
            throw std::invalid_argument("Cannot delete a file");
        }

        history.top()->deleteChild(name);
        overwriteFile(history.top());
    }
    void copyFile(std::string name, Directory* destination) {
        BaseObject* toCopy = history.top()->getChild(name);

        if (toCopy->getType() != FILETYPE) {
            throw std::invalid_argument("Cannot copy a directory");
        }


    }
    void printFileContent(std::string name, std::ostream& out = std::cout) {
        BaseObject* file = history.top()->getChild(name);
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

int main() {
    bool restart;
    std::cout << "Do you want to restart the file system? (1/0)" << std::endl;
    std::cin >> restart;
    if (restart) {
        File essay(0, "essay.txt", "This is an essay");
        Directory root(0, "root");
        Directory photos(0, "photos");
        root.addChild(&essay);
        root.addChild(&photos);
        File photo1(0, "photo1.jpg", "This is a photo");
        photos.addChild(&photo1);
        root.print();
        std::fstream fileStream("fileSystem.bat", std::ios::out | std::ios::binary);
        root.serialize(fileStream);
        fileStream.close();
    }

    // std::fstream fileStream("fileSystem.bat", std::ios::in | std::ios::binary);
    // FileSystemObjectType type;
    // fileStream.read(reinterpret_cast<char*>(&type), sizeof(FileSystemObjectType));
    // Directory* root = Directory::deserializeShallow(fileStream);
    // root->print();
    // fileStream.close();


    FileSystemManager manager("fileSystem.bat");
    std::string command;
    do {
        std::cin >> command;
        try {
            if (command == "ls") {
                manager.printCurrentDirectory();
            } else if (command == "cd") {
                std::string name;
                std::cin >> name;
                if (name == "..") {
                    manager.goBack();
                } else {
                    manager.goIn(name);
                }
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

            } else if (command == "exit") {
                break;
            } else {
                std::cout << command << " is not a recognized command" << std::endl;
            }
        } catch (std::invalid_argument& e) {
                std::cout << e.what() << std::endl;
            }
    } while (true);
    return 0;
}