#pragma once

#include "directory.h"
#include <stack>

class FileSystemManager {
private:
    std::stack<Directory*> history;
    std::string filename;

    void overwriteFile(Directory* current);

public:
    // Constructor
    FileSystemManager(std::string path);

    // Navigation methods
    void goIn(std::string name);
    void goBack();
    void manageMoveCommand(std::string command);

    // File system operations
    void createDirectory(std::string name);
    void deleteDirectory(std::string name);
    void deleteFile(std::string name);
    void copyFile(std::string filename, std::string destinationPath);
    void printFileContent(std::string name, std::ostream& out = std::cout);

    // Utility methods
    void serialize(std::fstream& fileStream);
    void printCurrentDirectory();
};