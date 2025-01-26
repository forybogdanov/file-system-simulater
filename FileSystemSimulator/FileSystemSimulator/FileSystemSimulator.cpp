#include "FileSystemManager.h"

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
    FileSystemManager manager(filename + ".bin");
    std::string command;
    do {
        std::cin >> command;
        try {
            if (command == "ls") {
                manager.printCurrentDirectory();

            }
            else if (command == "cd") {
                std::string name;
                std::cin >> name;
                manager.manageMoveCommandMultiple(name);

            }
            else if (command == "mkdir") {
                std::string name;
                std::cin >> name;
                manager.createDirectory(name);

            }
            else if (command == "rmdir") {
                std::string name;
                std::cin >> name;
                manager.deleteDirectory(name);

            }
            else if (command == "rm") {
                std::string name;
                std::cin >> name;
                manager.deleteFile(name);

            }
            else if (command == "cat") {
                std::string name;
                std::cin >> name;
                manager.printFileContent(name);

            }
            else if (command == "cp") {
                std::string name, destination;
                std::cin >> name >> destination;
                manager.copyFile(name, destination);

            }
            else if (command == "write") {
                std::string name, content;
                std::cin >> name;
                std::getline(std::cin, content);
                manager.writeToFile(name, content);

            }
            else if (command == "import") {
                std::string source, destination;
                std::cin >> source >> destination;
                std::string append;
                std::getline(std::cin, append);
                manager.importFile(source, destination, append);

            }
            else if (command == "export") {
                std::string source, destination;
                std::cin >> source >> destination;
                manager.exportFile(source, destination);

            }
            else if (command == "help") {
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
            else if (command == "createExample") {
                createExample();

            }
            else if (command == "exit") {
                break;
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