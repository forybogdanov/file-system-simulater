#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<exception>
#include<stack>

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
    BaseObject(FileSystemObjectType t, std::string name) {
        this->name = name;
        this->type = t;
    }
    BaseObject(FileSystemObjectType t, std::string name, BaseObject* parent) {
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
    File(std::string name, std::string content) : BaseObject(FILETYPE, name) {
        this->content = content;
    }
    File(std::string name, BaseObject* parent, std::string content) : BaseObject(FILETYPE, name, parent) {
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

        std::string name = readString(fileStream);
        std::string content = readString(fileStream);

        return new File(name, content);
    }
    void print(std::ostream& out = std::cout, int nestedDegree = 0) override {
        printNestness(out, nestedDegree);
        out << "(File)" << name <<  ": " << content << std::endl;
    }
};

class Directory : public BaseObject {
    private:
    std::vector<BaseObject*> children;
    public:
    Directory(std::string name) : BaseObject(DIRTYPE, name) {}
    Directory(std::string name, BaseObject* parent) : BaseObject(DIRTYPE, name, parent) {}
    void addChild(BaseObject* child) {
        children.push_back(child);
    }
    void serialize(std::fstream& fileStream) {


        if (!fileStream.is_open()) {
            std::ios_base::failure("File not found");
        }

        BaseObject::serialize(fileStream);

        std::size_t childrenCount = children.size();

        fileStream.write(reinterpret_cast<char const *>(&childrenCount), sizeof(std::size_t));

        for(auto child : children) {
            if (child->getType() == DIRTYPE) {
                Directory* dir = dynamic_cast<Directory*>(child);
                dir->serialize(fileStream);
            } else {
                File* file = dynamic_cast<File*>(child);
                file->serialize(fileStream);
            }
        }
    }
    static Directory* deserializeFull(std::fstream& fileStream) {

        if (!fileStream.is_open()) {
            std::ios_base::failure("File not found");
        }

        std::string name = readString(fileStream);
        Directory* dir = new Directory(name);

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

        std::string name = readString(fileStream);
        Directory* dir = new Directory(name);

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
        out << "(Directory)" << name << ":" << std::endl;
        for(auto child : children) {
            child->print(out, nestedDegree+1);
        }
    }
};

class FileSystemManager {
    private:
    Directory* root;
    public:
    FileSystemManager(std::string path) {
        std::fstream fileStream(path);

        if(!fileStream.is_open()) {
           std::invalid_argument("File not found");
        }

        FileSystemObjectType type;
        fileStream.read(reinterpret_cast<char*>(&type), sizeof(FileSystemObjectType));
        
        if (type == FILETYPE) {
            std::invalid_argument("Root cannot be a file");
        }

        root = Directory::deserializeShallow(fileStream);

        fileStream.close();
    }
    ~FileSystemManager() {
        delete root;
    }
    void serialize(std::fstream& fileStream) {
        root->serialize(fileStream);
    }
    void printCurrentDirectory() {
        root->print();
    }
   
};

int main() {
    // File essay("essay.txt", "This is an essay");
    // Directory root("root");
    // Directory photos("photos");
    // root.addChild(&essay);
    // root.addChild(&photos);
    // File photo1("photo1.jpg", "This is a photo");
    // photos.addChild(&photo1);
    // root.print();
    // std::fstream fileStream("fileSystem.bat", std::ios::out | std::ios::binary);
    // root.serialize(fileStream);
    // fileStream.close();

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
        if (command == "ls") {
            manager.printCurrentDirectory();
        }
    } while (command != "exit");
    return 0;
}