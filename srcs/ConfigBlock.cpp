#include "ConfigBlock.hpp"

#include <cstdlib>
#include <sstream>

#include "Error.hpp"
#include "Parser.hpp"

static unsigned int strToIp(std::string ip) {
    if (ip == "localhost")
        ip = "127.0.0.1";

    std::istringstream ss(ip);
    std::string octet_str;
    std::vector<unsigned int> octets;

    while (std::getline(ss, octet_str, '.')) {
        unsigned int octet;
        if (!(std::istringstream(octet_str) >> octet) || octet > 255) {
            throw Error("Invalid host");
            return 0;
        }
        octets.push_back(octet);
    }

    if (octets.size() != 4) {
        throw Error("Invalid host");
        return 0;
    }

    return ((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
}

ConfigBlock::ConfigBlock(std::string& fileContent) : _maxBodySize(false, 1024 * 1024) /* 1MB */, _root(false, "./") {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word != "{")
        throw Error("Expected '{'");

    while (1) {
        Parser::extractWord(fileContent, word);
        if (word.empty())
            break;

        if (word == "client_max_body_size")
            this->setMaxBodySize(fileContent);
        else if (word == "root")
            this->setRoot(fileContent);
        else if (word == "listen")
            this->setListen(fileContent);
        else if (word == "server_name")
            this->setServerName(fileContent);
        else if (word == "error_page")
            this->setErrorPage(fileContent);
        else if (word == "location")
            this->setLocation(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

ConfigBlock::~ConfigBlock() {
}

size_t ConfigBlock::getMaxBodySize(void) const {
    return this->_maxBodySize.second;
}

const std::string& ConfigBlock::getRoot(void) const {
    return this->_root.second;
}

const std::vector<t_listen>& ConfigBlock::getListen(void) const {
    return this->_listen;
}

std::vector<std::string>& ConfigBlock::getServerName(void) {
    return this->_serverNames;
}

const std::string* ConfigBlock::getErrorPage(int errorCode) const {
    if (this->_errorPages.count(errorCode) == 0)
        return NULL;
    return &this->_errorPages.at(errorCode);
}

const Location* ConfigBlock::getLocation(std::string uri) const {
    while (1) {
        if (this->_locations.count(uri) != 0)
            return &this->_locations.at(uri);

        if (uri[uri.length() - 1] == '/')
            uri.erase(uri.length() - 1);
        else {
            size_t pos = uri.find_last_of('/');
            if (pos == std::string::npos)
                return NULL;
            uri.erase(pos + 1);
        }
    }
}

void ConfigBlock::setMaxBodySize(std::string& fileContent) {
    if (this->_maxBodySize.first == true)
        throw Error("Duplicated client_max_body_size");

    this->_maxBodySize.first = true;

    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    size_t i = 0;
    while (i < word.length() - 1) {
        if (std::isdigit(word[i]) == false)
            throw Error("Invalid client_max_body_size");
        i++;
    }

    if (word[i] == 'K')
        this->_maxBodySize.second = std::atoi(word.c_str()) * 1024;
    else if (word[i] == 'M')
        this->_maxBodySize.second = std::atoi(word.c_str()) * 1024 * 1024;
    else if (word[i] == 'G')
        this->_maxBodySize.second = std::atoi(word.c_str()) * 1024 * 1024 * 1024;
    else
        throw Error("Invalid client_max_body_size");

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void ConfigBlock::setRoot(std::string& fileContent) {
    if (this->_root.first == true)
        throw Error("Duplicated root");

    this->_root.first = true;

    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");
    if (word == ";" || word == "{" || word == "}")
        throw Error("Expected a root");

    if (word[0] != '.' && word[0] != '/')
        word.insert(0, "./");

    this->_root.second = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void ConfigBlock::setListen(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    unsigned int host = strToIp(word.substr(0, word.find(':')));

    word = word.substr(word.find(':') + 1, word.length() - word.find(':') - 1);
    if (word.empty())
        throw Error("You must provide a port");

    for (size_t i = 0; i < word.length(); i++) {
        if (std::isdigit(word[i]) == false)
            throw Error("Invalid server port");
    }

    int port = std::atoi(word.c_str());
    if (port < 1 || port > 65535)
        throw Error("Invalid server port");

    t_listen hostPort;
    hostPort.host = host;
    hostPort.port = port;

    for (size_t i = 0; i < this->_listen.size(); i++) {
        if (this->_listen[i].host == hostPort.host && this->_listen[i].port == hostPort.port)
            throw Error("Duplicated listen");
    }

    this->_listen.push_back(hostPort);

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void ConfigBlock::setServerName(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");
    if (word == ";" || word == "{" || word == "}")
        throw Error("Expected a server name");

    for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end(); it++) {
        if (*it == word)
            throw Error("Duplicated server name");
    }

    this->_serverNames.push_back(word);

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void ConfigBlock::setErrorPage(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    for (size_t i = 0; i < word.length(); i++) {
        if (std::isdigit(word[i]) == false)
            throw Error("Invalid error code");
    }

    int errorCode = std::atoi(word.c_str());
    if (errorCode < 100 || errorCode > 599)
        throw Error("Invalid error code");

    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    if (this->_errorPages.count(errorCode) != 0)
        throw Error("Duplicated error code");

    this->_errorPages[errorCode] = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void ConfigBlock::setLocation(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    for (size_t i = 0; i < word.length(); i++) {
        if (std::isalnum(word[i]) == false && word[i] != '_')
            throw Error("Invalid location: \"" + word + "\"");
    }

    if (this->_locations.count(word))
        throw Error("Duplicated location \"" + word + "\"");

    this->_locations[word].configure(word, fileContent);
}
