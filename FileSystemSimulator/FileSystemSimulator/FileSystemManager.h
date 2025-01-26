#pragma once

#include <stack>
#include "Directory.h"

class FileSystemManager {
private:
    std::stack<Directory*> history;
    std::string filename;

    void manageMoveCommand(std::string command);

public:
    FileSystemManager(std::string path);
    void goIn(std::string name);
    void goBack();
    void manageMoveCommandMultiple(std::string command);
    void overwriteFile(BaseObject* current);
    void createDirectory(std::string name);
    void deleteDirectory(std::string name);
    void deleteFile(std::string name);
    void copyFile(std::string filename, std::string destinationPath);
    void writeToFile(std::string name, std::string content);
    void importFile(std::string source, std::string destination, std::string append);
    void exportFile(std::string source, std::string destination);
    void printFileContent(std::string name, std::ostream& out = std::cout);
    void serialize(std::fstream& fileStream);
    void printCurrentDirectory();
};
