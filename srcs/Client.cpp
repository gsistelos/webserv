#include "Client.hpp"

#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>

#include "Error.hpp"

Client::Client(int serverFd) {
    // Accept client connection

    this->_addrlen = sizeof(this->_address);

    this->_socketFd = accept(serverFd, (sockaddr*)&this->_address, &this->_addrlen);
    if (this->_socketFd == -1)
        throw Error("Accept");

    // Get client address and port
    char clientIp[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &this->_address.sin_addr, clientIp, INET_ADDRSTRLEN) == NULL)
        throw Error("inet_ntop");

    int clientPort = ntohs(this->_address.sin_port);

    std::cout << "Accepted client: " << clientIp << ":" << clientPort << " on socketFd " << this->_socketFd << std::endl;
}

int Client::getSocketFd(void) {
    return this->_socketFd;
}

Client::~Client() {
    std::cout << "Client socketFd " << this->_socketFd << " closed" << std::endl;
    close(this->_socketFd);
}
