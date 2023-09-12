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

    std::vector<char> buffer(bodySize);

    ssize_t bytes = read(this->_fd, buffer.data(), bodySize);
    if (bytes == -1)
        throw Error("read");
    if (bytes == 0) {
        WebServ::removeIndex(index);
        return;
    }

    this->_request.assign(buffer.data(), bytes);

    std::cout << "Request: " << this->_request << std::endl;
    this->_headerEnd = this->_request.find("\r\n\r\n");
    if (this->_headerEnd == std::string::npos)
        this->_headerEnd = this->_request.length();

    std::string method;
    Parser::getWord(this->_request, method);

    try {
        if (method == "GET")
            getMethod();
        else if (method == "POST")
            postMethod();
        else if (method == "DELETE")
            deleteMethod();
        else
            this->getPage("HTTP/1.1 501 Not Implemented", this->_server->getRoot() + "default_pages/501.html");
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        this->internalServerError();
    }

    if (WebServ::pollFds[index].revents & POLLOUT) {
        bytes = write(this->_fd, this->_response.c_str(), this->_response.length());
        if (bytes == -1)
            throw Error("write");
        if (bytes == 0) {
            WebServ::removeIndex(index);
            return;
        }
    }
}

void Client::getMethod(void) {
    std::string uri;
    Parser::getWord(this->_request, uri, 4);

    if (this->isRedirect(uri))
        return;

    uri = this->_server->getRoot() + uri;
    if (uri[uri.length() - 1] == '/')
        uri += "index.html";

    if (access(uri.c_str(), F_OK))
        this->getPage("HTTP/1.1 404 Not Found", "default_pages/404.html");
    else
        this->getPage("HTTP/1.1 200 OK", uri);
}

void Client::postMethod(void) {
    Cgi cgi(this->_request, this->_headerEnd, this->_response);
    cgi.setCgiPath("cgi-bin/upload.py");

    cgi.pushEnv("REQUEST_METHOD=POST");
    cgi.pushEnvFromHeader("Content-Type: ", "CONTENT_TYPE=");

    cgi.execScript();
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}

void Client::getPage(const std::string& http, const std::string& uri) {
    try {
        std::string buffer;
        Parser::readFile(uri, buffer);

        std::stringstream responseStream;
        responseStream << http << "\r\n"
                       << "Content-Length: " << buffer.length() << "\r\n"
                       << "\r\n"
                       << buffer;

        this->_response = responseStream.str();
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        this->internalServerError();
    }
}

int Client::isRedirect(const std::string& uri) {
    if (uri == "/redirect")
        this->_response = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.google.com/\r\n\r\n";
    else
        return 0;
    return 1;
}

void Client::internalServerError(void) {
    std::stringstream buffer;
    buffer << "<html>\r\n"
           << "<head><title>500 Internal Server Error</title></head>\r\n"
           << "<body>\r\n"
           << "<h1>500 Internal Server Error</h1>\r\n"
           << "<p>The server encountered an unexpected condition that prevented it from fulfilling the request</p>\r\n"
           << "<a href=\"/\">Back to Home</a>\r\n"
           << "</body>\r\n"
           << "</html>\r\n";

    std::stringstream responseStream;
    responseStream << "HTTP/1.1 500 Internal Server Error\r\n"
                   << "Content-Length: " << buffer.str().length() << "\r\n"
                   << "\r\n"
                   << buffer.str();
    this->_response = responseStream.str();
}
