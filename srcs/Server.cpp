#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>

#include <iostream>

#include "Error.hpp"

#define MAX_CLIENTS 128

Server::Server(const std::string& serverIp, int serverPort) {
    // Create socket and set to non-block

    this->_socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_socketFd == -1)
        throw Error("socket");

    fcntl(this->_socketFd, F_SETFL, O_NONBLOCK);

    this->_addrlen = sizeof(this->_address);
    std::memset(&this->_address, 0, this->_addrlen);

    // Bind server socket to address and port

    this->_address.sin_family = AF_INET;
    this->_address.sin_port = htons(serverPort);

    // TODO: remove forbidden function inet_pton
    if (inet_pton(AF_INET, serverIp.c_str(), &this->_address.sin_addr) != 1)
        throw Error("Invalid server address");

    if (bind(this->_socketFd, (struct sockaddr*)&this->_address, this->_addrlen) == -1)
        throw Error("bind");

    // Set server address to listen for incoming connections

    if (listen(this->_socketFd, MAX_CLIENTS) == -1)
        throw Error("Listen");

    std::cout << "Created server: " << serverIp << ":" << serverPort << " on socketFd " << this->_socketFd << std::endl;
}

int Server::getSocketFd(void) {
    return this->_socketFd;
}

Server::~Server() {
    std::cout << "Server socketFd " << this->_socketFd << " closed" << std::endl;
    close(this->_socketFd);
}
