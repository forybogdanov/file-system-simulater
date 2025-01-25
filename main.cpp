#include "file_system_manager.h"

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
        std::fstream fileStream("fileSystem.bin", std::ios::out | std::ios::binary);
        root.serialize(fileStream);
        fileStream.close();
    }

    // for (int i = -10; i < 10; i++) {
    //     std::cout << i << " ";
    //     try {
    //         std::fstream fileStream("fileSystem.bin", std::ios::in | std::ios::binary);
    //         Directory* dir = Directory::deserializeShallowFrom(fileStream, 144+i);
    //         dir->print();
    //         fileStream.close();
    //         delete dir;
    //     } catch (std::exception& e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }

    // std::fstream fileStream("fileSystem.bin", std::ios::in | std::ios::binary);
    // Directory* root = Directory::deserializeFullFrom(fileStream, 0);
    // root->print();
    // fileStream.close();


    FileSystemManager manager("fileSystem.bin");
    std::string command;
    do {
        std::cin >> command;
        try {
            if (command == "ls") {
                manager.printCurrentDirectory();
            } else if (command == "cd") {
                std::string name;
                std::cin >> name;
                manager.manageMoveCommand(name);
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