#include "Client.hpp"

#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "Cgi.hpp"
#include "Error.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

Client::Client(Server* server) {
    this->_server = server;

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    this->_fd = accept(server->getFd(), (sockaddr*)&address, &addrlen);
    if (this->_fd == -1)
        throw Error("accept");

    // Get client address and port
    char clientIp[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &address.sin_addr, clientIp, INET_ADDRSTRLEN) == NULL)
        throw Error("inet_ntop");

    int clientPort = ntohs(address.sin_port);

    WebServ::sockets.push_back(this);
    WebServ::pushPollfd(this->_fd);

    std::cout << "Accepted client: " << clientIp << ":" << clientPort << " on fd " << this->_fd << std::endl;
}

Client::~Client() {
}

void Client::handlePollin(int index) {
    std::cout << "Incoming data from client fd: " << this->_fd << std::endl;

    size_t bodySize = this->_server->getMaxBodySize();

    char* buffer = new char[bodySize + 1];

    size_t bytesRead = read(this->_fd, buffer, bodySize);

    if (bytesRead == (size_t)-1) {
        std::cerr << "webserv: read: " << strerror(errno) << std::endl;
        WebServ::removeIndex(index);
        return;
    }
    if (bytesRead == 0) {
        WebServ::removeIndex(index);
        return;
    }

    buffer[bytesRead] = '\0';

    this->_request = buffer;
    delete[] buffer;

    std::string method;
    Parser::extractWord(this->_request, method);

    if (method == "GET")
        getMethod();
    else if (method == "POST")
        postMethod();
    else if (method == "DELETE")
        deleteMethod();
    else
        _response = "HTTP/1.1 400 Method Not Supported\r\n\r\n";

    if (WebServ::pollFds[index].revents & POLLOUT) {
        if (write(this->_fd, this->_response.c_str(), this->_response.length()) == -1) {
            std::cerr << "webserv: write: " << strerror(errno) << std::endl;
            WebServ::removeIndex(index);
        }
    }
}

void Client::getMethod(void) {
    Cgi getCgi(this->_request, this->_response);
    getCgi.pushArgv("cgi-bin/get.py");

    std::string uri;
    Parser::extractWord(this->_request, uri);

    uri = this->_server->getRoot() + uri;

    getCgi.pushEnv("REQUEST_URI=" + uri);

    getCgi.execScript();
    getCgi.buildResponse();
}

void Client::postMethod(void) {
    Cgi uploadCgi(this->_request, this->_response);
    uploadCgi.pushArgv("cgi-bin/upload.py");

    uploadCgi.execScript();
    uploadCgi.buildResponse();
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}
