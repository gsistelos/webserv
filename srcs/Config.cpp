#include "Config.hpp"

#include <cstdlib>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

Config::Config(void) : availableMethods(0), autoIndex(false), canUpload(false) {
}

Config::~Config() {
}

void Config::configure(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word != "{")
        throw Error("Expected '{'");

    while (1) {
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == "}")
            break;

        if (word == "available_methods")
            this->setAvailableMethods(fileContent);
        else if (word == "return")
            this->setRedirect(fileContent);
        else if (word == "alias")
            this->setAlias(fileContent);
        else if (word == "autoindex")
            this->setAutoIndex(fileContent);
        else if (word == "index")
            this->setIndex(fileContent);
        else if (word == "cgi_extension")
            this->setCgiExtensions(fileContent);
        else if (word == "can_upload")
            this->setCanUpload(fileContent);
        else if (word == "upload_path")
            this->setUploadPath(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

void Config::setAvailableMethods(std::string& fileContent) {
    while (1) {
        std::string word;
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == ";")
            break;

        if (word == "GET")
            this->availableMethods |= GET;
        else if (word == "POST")
            this->availableMethods |= POST;
        else if (word == "DELETE")
            this->availableMethods |= DELETE;
        else
            throw Error("Invalid available_methods");
    }
}

void Config::setRedirect(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an redirect route");

    this->redirect = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setAlias(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an alias route");

    this->alias = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setAutoIndex(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);

    if (word == "on")
        this->autoIndex = true;
    else if (word == "off")
        this->autoIndex = false;
    else
        throw Error("Invalid autoindex");

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setIndex(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an index route");

    this->index = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setCgiExtensions(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected a cgi_extension");

    this->cgiExtensions.push_back(word);

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setCanUpload(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);

    if (word == "on")
        this->canUpload = true;
    else if (word == "off")
        this->canUpload = false;
    else
        throw Error("Invalid can_upload");

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Config::setUploadPath(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an upload_path route");

    this->uploadPath = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}
