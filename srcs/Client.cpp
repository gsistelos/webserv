#include "Client.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

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

    WebServ::sockets.push_back(this);
    WebServ::pushPollfd(this->_fd);

    // Get client address and port

    char clientIp[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &address.sin_addr, clientIp, INET_ADDRSTRLEN) == NULL)
        throw Error("inet_ntop");

    int clientPort = ntohs(address.sin_port);

    std::cout << "Accepted client: " << clientIp << ":" << clientPort << " on fd " << this->_fd << std::endl;
}

Client::~Client() {
}

void Client::handlePollin(int index) {
    std::cout << "Incoming data from client fd: " << this->_fd << std::endl;

    size_t bodySize = this->_server->getMaxBodySize();

    char buffer[bodySize];

    ssize_t bytesRead = read(this->_fd, buffer, bodySize);
    if (bytesRead <= 0) {
        WebServ::removeIndex(index);
        if (bytesRead == -1)
            std::cerr << "webserv: read: " << strerror(errno) << std::endl;
        return;
    }

    std::string request(buffer, bytesRead);

    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        this->_header = request;
        this->_body = "";
    } else {
        this->_header = request.substr(0, headerEnd);
        this->_body = request.substr(headerEnd + 4);
    }

    std::string method;
    Parser::extractWord(this->_header, method);

    try {
        if (method == "GET")
            getMethod();
        else if (method == "POST")
            postMethod();
        else if (method == "DELETE")
            deleteMethod();
        else
            invalidMethod();
    } catch (std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        WebServ::removeIndex(index);
        return;
    }

    if (WebServ::pollFds[index].revents & POLLOUT) {
        if (write(this->_fd, this->_response.c_str(), this->_response.length()) == -1) {
            std::cerr << "webserv: write: " << strerror(errno) << std::endl;
            WebServ::removeIndex(index);
        }
    }
}

void Client::getMethod(void) {
    std::string uri;
    Parser::extractWord(this->_header, uri);

    uri = this->_server->getRoot() + uri;
    if (uri[uri.length() - 1] == '/')
        uri += "index.html";

    std::stringstream responseStream;

    if (uri == "/redirect") {
        responseStream << "HTTP/1.1 301 Moved Permanently\r\n"
                       << "Location: http://www.google.com/\r\n"
                       << "\r\n";
    } else {
        std::string buffer;
        Parser::readFile(uri, buffer);

        responseStream << "HTTP/1.1 200 OK\r\n"
                       << "Content-Length: " << buffer.length() << "\r\n"
                       << "Content-Type: text/html\r\n"
                       << "\r\n"
                       << buffer;
    }

    this->_response = responseStream.str();
}

void Client::postMethod(void) {
    Cgi cgi(this->_header, this->_body, this->_response);
    cgi.setCgiPath("cgi-bin/upload.py");

    cgi.pushEnv("REQUEST_METHOD=POST");
    cgi.pushEnvFromHeader("Content-Type", "CONTENT_TYPE");

    cgi.execScript();
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}

void Client::invalidMethod(void) {
    _response = "HTTP/1.1 400 Method not supported\r\n\r\n";
}
