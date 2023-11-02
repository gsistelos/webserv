#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "Client.hpp"
#include "Error.hpp"
#include "Parser.hpp"
#include "WebServ.hpp"

#define MAX_CLIENTS 128

Server::Server(t_listen hostPort) {
    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd == -1)
        throw Error("socket");

    int opt = 1;

    if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
        throw Error("setsockopt");

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(hostPort.host);
    addr.sin_port = htons(hostPort.port);

    if (bind(this->_fd, (sockaddr*)&addr, sizeof(addr)) != 0)
        throw Error("bind");

    if (listen(this->_fd, MAX_CLIENTS) == -1)
        throw Error("listen");

    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK) == -1)
        throw Error("fcntl");

    std::cout << "Listening on localhost:" << hostPort.port << std::endl;
}

Server::~Server() {
}

const ConfigBlock& Server::getConfig(const std::string& serverName) {
    if (serverName.empty())
        return this->_configBlocks[0];

    for (std::vector<ConfigBlock>::iterator it = this->_configBlocks.begin(); it != this->_configBlocks.end(); it++) {
        std::vector<std::string> serverNames = it->getServerName();

        std::vector<std::string>::iterator found = std::find(serverNames.begin(), serverNames.end(), serverName);

        if (found != serverNames.end())
            return *it;
    }

    return this->_configBlocks[0];
}

void Server::push_back(ConfigBlock& configBlock) {
    std::vector<std::string> serverNames1 = configBlock.getServerName();

    for (std::vector<ConfigBlock>::iterator it = this->_configBlocks.begin(); it != this->_configBlocks.end(); it++) {
        std::vector<std::string> serverNames2 = it->getServerName();

        for (size_t i = 0; i < serverNames1.size(); i++) {
            std::vector<std::string>::iterator found = std::find(serverNames2.begin(), serverNames2.end(), serverNames1[i]);

            if (found != serverNames2.end()) {
                std::cerr << "Warning: server name " << serverNames1[i] << " already exists, ignoring" << std::endl;
                serverNames1.erase(serverNames1.begin() + i);
            }
        }
    }

    this->_configBlocks.push_back(configBlock);
}

void Server::handlePollin(int index) {
    (void)index;

    try {
        new Client(*this);
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
    }
}

void Server::handlePollout(int index) {
    (void)index;
}
