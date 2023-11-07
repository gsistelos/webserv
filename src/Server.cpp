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

Server::Server(t_listen hostPort, ConfigBlock& configBlock) : _hostPort(hostPort), _configDefault(&configBlock) {
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

    this->configToServerName(configBlock);
    WebServ::push_back(this);
    std::cout << "Listening on localhost:" << hostPort.port << std::endl;
}

void Server::configToServerName(ConfigBlock& configBlock) {
    std::vector<std::string> serverNames = configBlock.getServerName();

    for (std::vector<std::string>::iterator it = serverNames.begin(); it != serverNames.end(); it++) {
        std::map<std::string, ConfigBlock*>::iterator found = this->_configs.find(*it);

        if (found != this->_configs.end()) {
            std::cerr << "webserv: [warn] conflicting server name \"" << *it << "\" on localhost:" << this->_hostPort.port << ", ignored" << std::endl;
            continue;
        }

        this->_configs[*it] = &configBlock;
    }
}

Server::~Server() {
}

bool Server::operator==(const t_listen& hostPort) const {
    return (this->_hostPort.host == hostPort.host && this->_hostPort.port == hostPort.port);
}

const ConfigBlock& Server::getConfig(const std::string& serverName) {
    if (this->_configs.count(serverName) != 0) {
        return *this->_configs[serverName];
    }

    return *this->_configDefault;
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
