#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Client.hpp"
#include "Error.hpp"
#include "Parser.hpp"
#include "WebServ.hpp"

#define MAX_CLIENTS 128

Server::Server(std::string& fileContent) {
    this->configure(fileContent);

    // Create socket and set to non-block

    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd == -1)
        throw Error("socket");

    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK))
        throw Error("fcntl");

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    // Bind server socket to address and port

    address.sin_family = AF_INET;
    address.sin_port = htons(this->_port);

    // TODO: remove forbidden function inet_pton
    if (inet_pton(AF_INET, this->_ip.c_str(), &address.sin_addr) != 1)
        throw Error("inet_pton");

    if (bind(this->_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
        throw Error("bind");

    // Set server address to listen for incoming connections

    if (::listen(this->_fd, MAX_CLIENTS) == -1)
        throw Error("listen");

    std::cout << "Created server: " << this->_ip << ":" << this->_port << " on fd " << this->_fd << std::endl;
}

Server::~Server() {
}

const std::string& Server::getRoot(void) {
    return this->_root;
}

size_t Server::getMaxBodySize(void) {
    return this->_maxBodySize;
}

void Server::handlePollin(int index) {
    (void)index;

    Client* client;

    try {
        client = new Client(this);
        WebServ::sockets.push_back(client);
    } catch (std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        return;
    }

    pollfd pollFd;
    pollFd.fd = client->getFd();
    pollFd.events = POLLIN | POLLOUT;
    pollFd.revents = 0;
    WebServ::pollFds.push_back(pollFd);
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
