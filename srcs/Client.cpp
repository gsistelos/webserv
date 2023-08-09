#include "Client.hpp"

#include <unistd.h>

#include <iostream>

#include "Error.hpp"

Client::Client(int serverFd) {
    this->_socketFd = accept(serverFd, (sockaddr *)&this->_address, &this->_addrlen);
    if (this->_socketFd == -1)
        throw Error("Accept");
    std::cout << "New Client socketfd: " << this->_socketFd << std::endl;
}

int Client::getSocketFd(void) {
    return this->_socketFd;
}

Client::~Client() {
    std::cout << "Client fd: " << this->_socketFd << " closed" << std::endl;
    close(this->_socketFd);
}
