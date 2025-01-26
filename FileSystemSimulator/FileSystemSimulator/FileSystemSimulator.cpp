#include "FileSystemManager.h"
#include <functional>

// Define the functions outside the main scope
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

void listDirectory(FileSystemManager& manager) {
    manager.printCurrentDirectory();
}

void changeDirectory(FileSystemManager& manager) {
    std::string name;
    std::cin >> name;
    manager.manageMoveCommandMultiple(name);
}

void makeDirectory(FileSystemManager& manager) {
    std::string name;
    std::cin >> name;
    manager.createDirectory(name);
}

void removeDirectory(FileSystemManager& manager) {
    std::string name;
    std::cin >> name;
    manager.deleteDirectory(name);
}

void removeFile(FileSystemManager& manager) {
    std::string name;
    std::cin >> name;
    manager.deleteFile(name);
}

void printFile(FileSystemManager& manager) {
    std::string name;
    std::cin >> name;
    manager.printFileContent(name);
}

void copyFile(FileSystemManager& manager) {
    std::string name, destination;
    std::cin >> name >> destination;
    manager.copyFile(name, destination);
}

void writeToFile(FileSystemManager& manager) {
    std::string name, content;
    std::cin >> name;
    std::getline(std::cin, content);
    manager.writeToFile(name, content);
}

void importFile(FileSystemManager& manager) {
    std::string source, destination;
    std::cin >> source >> destination;
    std::string append;
    std::getline(std::cin, append);
    manager.importFile(source, destination, append);
}

void exportFile(FileSystemManager& manager) {
    std::string source, destination;
    std::cin >> source >> destination;
    manager.exportFile(source, destination);
}

void printHelp() {
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
}


int main() {
    std::string filename;
    std::cout << "Enter the file system file name: ";
    std::cin >> filename;
    FileSystemManager manager(filename + ".bin");

    std::unordered_map<std::string, std::function<void()>> commandMap = {
        {"ls", [&manager]() { listDirectory(manager); }},
        {"cd", [&manager]() { changeDirectory(manager); }},
        {"mkdir", [&manager]() { makeDirectory(manager); }},
        {"rmdir", [&manager]() { removeDirectory(manager); }},
        {"rm", [&manager]() { removeFile(manager); }},
        {"cat", [&manager]() { printFile(manager); }},
        {"cp", [&manager]() { copyFile(manager); }},
        {"write", [&manager]() { writeToFile(manager); }},
        {"import", [&manager]() { importFile(manager); }},
        {"export", [&manager]() { exportFile(manager); }},
        {"help", printHelp},
        {"createExample", createExample},
        {"exit",[&manager]() {}}
    };

    std::string command;
    do {
        std::cin >> command;
        try {
            auto it = commandMap.find(command);
            if (command == "exit") break;
            if (it != commandMap.end()) {
                it->second();
            }
            else {
                std::cout << command << " is not a recognized command" << std::endl;
            }
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    } while (true);
    return 0;
}