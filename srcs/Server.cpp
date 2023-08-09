#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include "Error.hpp"
#define MAX_CLIENTS 128

Server::Server(const std::string& serverAddr, int port) {
    std::cout << "Server addr: " << serverAddr << " Port: " << port << std::endl;
    this->_socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_socketFd == -1)
        throw Error("socket");

    std::cout << "Server fd: " << this->_socketFd << std::endl;
    // Set to non-block
    fcntl(this->_socketFd, F_SETFL, O_NONBLOCK);

    this->_addrlen = sizeof(this->_address);
    std::memset(&this->_address, 0, this->_addrlen);

    // Bind server address and port to socket

    this->_address.sin_family = AF_INET;
    this->_address.sin_port = htons(port);

    if (inet_pton(AF_INET, serverAddr.c_str(), &this->_address.sin_addr) != 1)
        throw Error("Invalid server address");

    // Set server address and to listen for incoming connections

    if (bind(this->_socketFd, (struct sockaddr*)&this->_address, this->_addrlen) == -1)
        throw Error("bind");

    if (listen(this->_socketFd, MAX_CLIENTS) == -1)
        throw Error("Listen");
}

int Server::getSocketFd(void) {
    return this->_socketFd;
}

Server::~Server() {
    close(this->_socketFd);
}
