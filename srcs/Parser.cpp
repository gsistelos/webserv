#include "Parser.hpp"

#include <fstream>

#include "Error.hpp"

void Parser::readFile(const std::string& filename, std::string& buf) {
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw Error(filename);

    buf = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void Parser::getWord(const std::string& str, std::string& buf, size_t start) {
    while (1) {
        start = str.find_first_not_of(" \t\r\n", start);
        if (start == std::string::npos) {
            buf.clear();
            return;
        }

        if (str[start] == '#') {
            start = str.find_first_of("\r\n", start);
            if (start == std::string::npos) {
                buf.clear();
                return;
            }
        } else
            break;
    }

    if (str[start] == '{' || str[start] == '}' || str[start] == ';') {
        buf = str[start];
        return;
    }

    size_t end = str.find_first_of(" \t\r\n{};", start);
    if (end == std::string::npos) {
        buf = str.substr(start);
        return;
    }

    buf = str.substr(start, end - start);
}

void Parser::extractWord(std::string& str, std::string& buf, size_t start) {
    while (1) {
        start = str.find_first_not_of(" \t\r\n", start);
        if (start == std::string::npos) {
            buf.clear();
            str.clear();
            return;
        }

        if (str[start] == '#') {
            start = str.find_first_of("\r\n", start);
            if (start == std::string::npos) {
                buf.clear();
                str.clear();
                return;
            }
        } else
            break;
    }

    if (str[start] == '{' || str[start] == '}' || str[start] == ';') {
        buf = str[start];
        str.erase(0, start + 1);
        return;
    }

    size_t end = str.find_first_of(" \t\r\n{};", start);
    if (end == std::string::npos) {
        buf = str.substr(start);
        str.clear();
        return;
    }

    buf = str.substr(start, end - start);
    str.erase(0, end);
}
