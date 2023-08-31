#include "Config.hpp"

#include <cstdlib>

#include "Error.hpp"
#include "Parser.hpp"

Config::Config(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word != "{")
        throw Error("Expecterd '{'");

    while (1) {
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == "}")
            break;

        // TODO: handle all configuration options

        if (word == "listen")
            this->setListen(fileContent);
        else if (word == "root")
            this->setRoot(fileContent);
        else if (word == "client_max_body_size")
            this->setMaxBodySize(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

Config::~Config() {
}

void Config::setListen(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    size_t colon = word.find_first_of(":");
    if (colon == std::string::npos)
        this->ip = "127.0.0.1";
    else
        this->ip = word.substr(0, colon);

    std::string serverPort = word.substr(colon + 1);
    for (size_t i = 0; i < serverPort.length(); i++) {
        if (!std::isdigit(serverPort[i]))
            throw Error("Invalid server port");
    }

    this->port = std::atoi(serverPort.c_str());
    if (this->port < 0 || this->port > 65535)
        throw Error("Invalid server port");

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setRoot(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    if (word[0] != '.' && word[0] != '/')
        word.insert(0, "./");

    this->root = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setMaxBodySize(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    size_t i = 0;
    while (i < word.length() - 1) {
        if (!std::isdigit(word[i]))
            throw Error("Invalid client_max_body_size");
        i++;
    }

    if (word[i] != 'M')
        throw Error("Invalid client_max_body_size");

    this->maxBodySize = std::atoi(word.c_str()) * 1024 * 1024;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}
