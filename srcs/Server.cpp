#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

#define MAX_CLIENTS 128

Server::Server(std::string& fileContent) {
    this->configure(fileContent);

    // Create socket and set to non-block

    this->_socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_socketFd == -1)
        throw Error("socket");

    fcntl(this->_socketFd, F_SETFL, O_NONBLOCK);

    this->_addrlen = sizeof(this->_address);
    std::memset(&this->_address, 0, this->_addrlen);

    // Bind server socket to address and port

    this->_address.sin_family = AF_INET;
    this->_address.sin_port = htons(this->_port);

    // TODO: remove forbidden function inet_pton
    if (inet_pton(AF_INET, this->_ip.c_str(), &this->_address.sin_addr) != 1)
        throw Error("Invalid server address");

    if (bind(this->_socketFd, (struct sockaddr*)&this->_address, this->_addrlen) == -1)
        throw Error("bind");

    // Set server address to listen for incoming connections

    if (::listen(this->_socketFd, MAX_CLIENTS) == -1)
        throw Error("Listen");

    std::cout << "Created server: " << this->_ip << ":" << this->_port << " on socketFd " << this->_socketFd << std::endl;
}

int Server::getSocketFd(void) {
    return this->_socketFd;
}

const std::string& Server::getRoot(void) {
    return this->_root;
}

std::string Server::readClientData(int clientFd) {
    std::cout << "Incoming data from client socketFd: " << clientFd << std::endl;

    char readBuffer[this->_maxBodySize + 1];

    size_t bytesRead = read(clientFd, readBuffer, this->_maxBodySize);

    if (bytesRead == (size_t)-1)
        throw Error("Read");

    readBuffer[bytesRead] = '\0';

    return readBuffer;
}

void Server::configure(std::string& fileContent) {
    std::string word = Parser::extractWord(fileContent);
    if (word != "{")
        throw Error("Expecterd '{'");

    while (1) {
        word = Parser::extractWord(fileContent);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == "}")
            break;

        // TODO: handle all configuration options

        if (word == "listen")
            this->listen(fileContent);
        else if (word == "root")
            this->root(fileContent);
        else if (word == "client_max_body_size")
            this->maxBodySize(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

void Server::listen(std::string& fileContent) {
    std::string word = Parser::extractWord(fileContent);
    if (word.empty())
        throw Error("Unexpected end of file");

    size_t colon = word.find_first_of(":");
    if (colon == std::string::npos)
        this->_ip = "127.0.0.1";
    else
        this->_ip = word.substr(0, colon);

    std::string serverPort = word.substr(colon + 1);
    for (size_t i = 0; i < serverPort.length(); i++) {
        if (!std::isdigit(serverPort[i]))
            throw Error("Invalid server port");
    }

    this->_port = std::atoi(serverPort.c_str());
    if (this->_port < 0 || this->_port > 65535)
        throw Error("Invalid server port");

    word = Parser::extractWord(fileContent);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::root(std::string& fileContent) {
    std::string word = Parser::extractWord(fileContent);
    if (word.empty())
        throw Error("Unexpected end of file");

    if (word[0] != '.' && word[0] != '/')
        word.insert(0, "./");

    this->_root = word;

    word = Parser::extractWord(fileContent);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::maxBodySize(std::string& fileContent) {
    std::string word = Parser::extractWord(fileContent);
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

    this->_maxBodySize = std::atoi(word.c_str()) * 1024 * 1024;

    word = Parser::extractWord(fileContent);
    if (word != ";")
        throw Error("Expected ';'");
}

Server::~Server() {
    std::cout << "Server socketFd " << this->_socketFd << " closed" << std::endl;
    close(this->_socketFd);
}
