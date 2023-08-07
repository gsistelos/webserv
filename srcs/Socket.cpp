#include "Socket.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <cstring>

#include "Error.hpp"

#define MAX_CLIENTS 128

Socket::Socket(std::stringstream &fileStream, std::vector<pollfd> &pollFds) : _type(SERVER) {
    std::string token;
    fileStream >> token;

    if (fileStream.fail())
        throw Error("Invalid content next to \"server:\"");

    // Extract server address and port

    size_t colon = token.find(':');
    if (colon == std::string::npos)
        throw Error("Invalid content next to \"server:\"");

    std::string server_addr = token.substr(0, colon);
    std::string portStr = token.substr(colon + 1, token.length());

    int         port = std::atoi(portStr.c_str());

    // Start server as TCP/IP and set to non-block

    this->_socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_socketFd == -1)
        throw Error("socket");

    fcntl(this->_socketFd, F_SETFL, O_NONBLOCK);

    this->_addrlen = sizeof(this->_address);
    std::memset(&this->_address, 0, this->_addrlen);

    // Bind server address and port to socket

    this->_address.sin_family = AF_INET;
    this->_address.sin_port = htons(port);
    if (inet_pton(AF_INET, server_addr.c_str(), &this->_address.sin_addr) != 1)
        throw Error("Invalid server address");

    // Set server address and to listen for incoming connections

    if (bind(this->_socketFd, (struct sockaddr *)&this->_address, this->_addrlen) == -1)
        throw Error("bind");

    if (listen(this->_socketFd, MAX_CLIENTS) == -1)
        throw Error("Listen");

    // Add server to pollfd vector

    pollfd pollFd;
    pollFd.fd = this->_socketFd;
    pollFd.events = POLLIN | POLLOUT;

    pollFds.push_back(pollFd);
}

Socket::Socket(int serverFd, std::vector<pollfd> &pollFds) : _type(CLIENT) {
    this->_socketFd = accept(serverFd, (sockaddr*)&this->_address, &this->_addrlen);
    if (this->_socketFd == -1)
        throw Error("Accept");

    pollfd pollFd;
    pollFd.fd = this->_socketFd;
    pollFd.events = POLLIN | POLLOUT;

    pollFds.push_back(pollFd);
}

Socket::~Socket() {
    close(this->_socketFd);
}

Type Socket::getType(void) {
    return this->_type;
}
