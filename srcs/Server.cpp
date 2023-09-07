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

Server::Server(std::string& fileContent) : _config(fileContent) {
    // Create socket and set to non-block

    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd == -1)
        throw Error("socket");

    int option = 1;

    if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) == -1)
        throw Error("setsockopt");

    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC))
        throw Error("fcntl");

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    // Bind server socket to address and port

    address.sin_family = AF_INET;
    address.sin_port = htons(this->_config.port);

    // TODO: remove forbidden function inet_pton
    if (inet_pton(AF_INET, this->_config.ip.c_str(), &address.sin_addr) != 1)
        throw Error("inet_pton");

    if (bind(this->_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
        throw Error("bind");

    // Set server address to listen for incoming connections

    if (listen(this->_fd, MAX_CLIENTS) == -1)
        throw Error("listen");

    WebServ::sockets.push_back(this);
    WebServ::pushPollfd(this->_fd);

    std::cout << "Created server: " << this->_config.ip << ":" << this->_config.port << " on fd " << this->_fd << std::endl;
}

Server::~Server() {
}

const std::string& Server::getRoot(void) {
    return this->_config.root;
}

size_t Server::getMaxBodySize(void) {
    return this->_config.maxBodySize;
}

void Server::handlePollin(int index) {
    (void)index;

    try {
        new Client(this);
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
    }
}
