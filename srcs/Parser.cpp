#include "Parser.hpp"

#include "Error.hpp"

void Parser::readFile(const std::string& filename, std::string& buf) {
    std::ifstream file(filename.c_str());
    if (!file)
        throw Error(filename);

    buf = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
}

void Parser::extractWord(std::string& str, std::string& buf) {
    buf.clear();

    // Skip whitespaces and comments

    while (1) {
        size_t start = str.find_first_not_of(" \t\n");
        if (start == std::string::npos) {
            str.clear();
            return;
        }

        str.erase(0, start);

        if (str[0] == '#') {
            size_t end = str.find_first_of("\n");
            if (end == std::string::npos) {
                str.clear();
                return;
            }
            str.erase(0, end);
        } else
            break;
    }

    // Extract word

    if (str[0] == '{' || str[0] == '}' || str[0] == ';') {
        buf = str[0];
        str.erase(0, 1);
        return;
    }

    size_t end = str.find_first_of(" \t\n{};");
    if (end == std::string::npos) {
        buf = str;
        str.clear();
        return;
    }

    buf = str.substr(0, end);
    str.erase(0, end);
}
