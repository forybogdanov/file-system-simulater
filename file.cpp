#include "file.h"


void File::setContent(std::string content) {
    this->content = content;
}

std::string File::getContent() {
    return content;
}

File::File(std::streampos start, std::string name, std::string content) : BaseObject(start, FILETYPE, name) {
    this->content = content;
}

void File::serialize(std::fstream& fileStream) {

    BaseObject::serialize(fileStream);

    std::size_t contentLength = content.length();
    fileStream.write(reinterpret_cast<char const *>(&contentLength), sizeof(std::size_t));
    fileStream.write(content.c_str(), contentLength);
}

File* File::deserialize(std::fstream& fileStream) {

    if (!fileStream.is_open()) {
        std::ios_base::failure("File not found");
    }

    std::streampos start = fileStream.tellg();

    std::string name = readString(fileStream);
    std::string content = readString(fileStream);

    return new File(start, name, content);
}

void File::print(std::ostream& out, int nestedDegree) {
    printNestness(out, nestedDegree);
    out << "(File)" << name << std::endl;
}
