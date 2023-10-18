#include "Location.hpp"

#include <cstdlib>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

Location::Location(void) : allowMethods(0), root("./"), autoIndex(false), canUpload(false) {
}

Location::~Location() {
}

void Location::configure(std::string& fileContent) {
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

        if (word == "allow_methods")
            this->setAllowMethods(fileContent);
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

void Location::setAllowMethods(std::string& fileContent) {
    while (1) {
        std::string word;
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == ";")
            break;

        if (word != "GET" && word != "POST" && word != "DELETE")
            throw Error("Invalid available_methods");

        this->allowMethods.push_back(word);
    }
}

void Location::setRedirect(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an redirect route");

    this->redirect = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Location::setAlias(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an alias route");

    this->alias = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Location::setAutoIndex(std::string& fileContent) {
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

void Location::setIndex(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an index route");

    this->index = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Location::setCgiExtensions(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected a cgi_extension");

    this->cgiExtensions.push_back(word);

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Location::setCanUpload(std::string& fileContent) {
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

void Location::setUploadPath(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word == ";" || word == "{" || word == "}" || word.empty())
        throw Error("Expected an upload_path route");

    this->uploadPath = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}
