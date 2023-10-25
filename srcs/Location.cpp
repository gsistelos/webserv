#include "Location.hpp"

#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

Location::Location(void) : autoindex(false), index("index.html"), canUpload(false), uploadPath(".") {
}

Location::~Location() {
}

bool Location::isMethodAllowed(const std::string& method) const {
    if (allowedMethods.empty())
        return true;
    return allowedMethods.find(method) != std::string::npos;
}

const std::string* Location::getRedirect(void) const {
    if (this->redirect.empty())
        return NULL;
    return &this->redirect;
}

const std::string* Location::getAlias(void) const {
    if (this->alias.empty())
        return NULL;
    return &this->alias;
}

bool Location::getAutoindex(void) const {
    return this->autoindex;
}

const std::string& Location::getIndex(void) const {
    return this->index;
}

bool Location::isCgiExtension(const std::string& filename) const {
    size_t extensionPos = filename.find_last_of('.');
    if (extensionPos == std::string::npos)
        return false;

    std::string extension = filename.substr(extensionPos + 1);

    if (this->cgiExtensions.find(extension) == std::string::npos)
        return false;
    return true;
}

bool Location::getCanUpload(void) const {
    return this->canUpload;
}

const std::string& Location::getUploadPath(void) const {
    return this->uploadPath;
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
            this->setAutoindex(fileContent);
        else if (word == "index")
            this->setIndex(fileContent);
        else if (word == "cgi_extensions")
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
    size_t count = 0;
    while (1) {
        std::string word;
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == ";")
            break;

        if (word != "GET" && word != "POST" && word != "DELETE")
            throw Error("Invalid allow_methods");

        this->allowedMethods.append(word);
        this->allowedMethods.push_back(' ');
        count++;
    }

    if (count == 0)
        throw Error("Expected a method");
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

void Location::setAutoindex(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);

    if (word == "on")
        this->autoindex = true;
    else if (word == "off")
        this->autoindex = false;
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
        throw Error("Expected a index");

    this->index = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Location::setCgiExtensions(std::string& fileContent) {
    size_t count = 0;
    while (1) {
        std::string word;
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == ";")
            break;

        if (word != "py" && word != "php")
            throw Error("Invalid cgi_extensions");

        this->cgiExtensions.append(word);
        this->cgiExtensions.push_back(' ');
        count++;
    }

    if (count == 0)
        throw Error("Expected a cgi extension");
}

void Location::setCanUpload(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);

    if (word == "true")
        this->canUpload = true;
    else if (word == "false")
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
        throw Error("Expected a upload path");

    this->uploadPath = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}
