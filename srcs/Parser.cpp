#include "Parser.hpp"

#include "Error.hpp"

std::string Parser::readFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file)
        throw Error(filename);

    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return fileContent;
}

std::string Parser::extractWord(std::string& str) {
    // Skip whitespaces and comments

    while (1) {
        size_t start = str.find_first_not_of(" \t\n");
        if (start == std::string::npos) {
            str.clear();
            return "";
        }

        str.erase(0, start);

        if (str[0] == '#') {
            size_t end = str.find_first_of("\n");
            if (end == std::string::npos) {
                str.clear();
                return "";
            }
            str.erase(0, end);
        } else
            break;
    }

    if (str[0] == '{' || str[0] == '}' || str[0] == ';') {
        std::string word = str.substr(0, 1);
        str.erase(0, 1);
        return word;
    }

    size_t end = str.find_first_of(" \t\n{};");
    if (end == std::string::npos) {
        std::string word = str;
        str.clear();
        return word;
    }

    std::string word = str.substr(0, end);
    str.erase(0, end);
    return word;
}
