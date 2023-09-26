#include "Client.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>

#include "Cgi.hpp"
#include "Error.hpp"
#include "HttpResponse.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

#define BUFFER_SIZE 1024  // 1 KB

const std::string* Client::getRedirect(const std::string& key) {
    static std::map<std::string, std::string> redirect;

    if (redirect.empty()) {
        redirect["/redirect"] = "https://www.google.com";
        redirect["/"] = "http://127.0.0.1:4000/pages/";
    }

    if (redirect.count(key))
        return &redirect.at(key);
    return NULL;
}

Client::Client(Server* server) {
    this->_server = server;

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    this->_fd = accept(server->getFd(), (sockaddr*)&address, &addrlen);
    if (this->_fd == -1)
        throw Error("accept");

    // if (fcntl(this->_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC))
    //     throw Error("fcntl");

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

    // Read header

    char headerBuffer[BUFFER_SIZE];

    size_t bytes = read(this->_fd, headerBuffer, BUFFER_SIZE);
    if (bytes == (size_t)-1)
        throw Error("read");
    if (bytes == 0) {
        WebServ::removeIndex(index);
        return;
    }

    std::string request(headerBuffer, bytes);

    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        this->_header = request.substr(0, request.length());
        request.erase(0, request.length());
    } else {
        this->_header = request.substr(0, headerEnd);
        request.erase(0, headerEnd + 4);
    }

    // std::cout << "======= HEADER =======" << std::endl;
    // std::cout << this->_header << std::endl;
    // std::cout << "======================" << std::endl;

    // Read body

    size_t contentLength = 0;
    std::string contentLengthStr = this->getHeaderValue("Content-Length: ");
    if (contentLengthStr != "") {
        contentLength = std::strtol(contentLengthStr.c_str(), NULL, 10);
        if (errno == ERANGE)
            throw Error("strtol");  // return 400
        if (contentLength > this->_server->getMaxBodySize())
            throw Error("body size too large");  // return 413

        this->_body = request;

        contentLength -= this->_body.length();

        char* bodyBuffer = new char[contentLength];

        size_t bytesRead = 0;
        while (bytesRead < contentLength) {
            size_t bytes = read(this->_fd, bodyBuffer + bytesRead, contentLength - bytesRead);
            if (bytes == (size_t)-1)
                throw Error("read");
            bytesRead += bytes;
        }

        this->_body.insert(this->_body.length(), bodyBuffer, bytesRead);
        delete[] bodyBuffer;

        // std::cout << "======= BODY =======" << std::endl;
        // std::cout << this->_body << std::endl;
        // std::cout << "======================" << std::endl;
    }

    std::string method;
    Parser::getWord(this->_header, method);

    try {
        if (method == "GET")
            getMethod();
        else if (method == "POST")
            postMethod();
        else if (method == "DELETE")
            deleteMethod();
        else
            HttpResponse::pageResponse(405, "default_pages/405.html");
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        this->_response = HttpResponse::internalServerError;
    }

    // std::cout << "========== RESPONSE =========" << std::endl;
    // std::cout << this->_response << std::endl;
    // std::cout << "=============================" << std::endl;

    if (WebServ::pollFds[index].revents & POLLOUT) {
        size_t bytes = write(this->_fd, this->_response.c_str(), this->_response.length());
        if (bytes == (size_t)-1)
            throw Error("write");
        if (bytes == 0) {
            WebServ::removeIndex(index);
            return;
        }
    }
}

void Client::getMethod(void) {
    std::string uri;
    Parser::getWord(this->_header, uri, 3);

    const std::string* redirect = getRedirect(uri);

    if (redirect) {
        HttpResponse response;
        response.setStatusCode(301);
        response.setHeader("Content-Length", 0);
        response.setHeader("Location", *redirect);

        this->_response = response.toString();
        return;
    }

    uri = this->_server->getRoot() + uri;
    if (uri[uri.length() - 1] == '/')
        uri += "index.html";

    this->_response = HttpResponse::accessPage(uri);
}

void Client::postMethod(void) {
    Cgi cgi("cgi-bin/upload.py", this->_header, this->_body);

    cgi.setEnv("REQUEST_METHOD=POST");
    cgi.setEnv("TRANSFER_ENCODING=chunked");
    cgi.setEnvFromHeader("Content-Type: ", "CONTENT_TYPE=");
    cgi.setEnvFromHeader("Content-Length: ", "CONTENT_LENGTH=");

    this->_response = cgi.getResponse();
}

void Client::deleteMethod(void) {
    this->_response = HttpResponse::pageResponse(501, "default_pages/501.html");
}

std::string Client::getHeaderValue(const std::string& header) {
    size_t headerStart = this->_header.find(header);
    if (headerStart == std::string::npos)
        return "";

    headerStart += header.length();
    size_t headerEnd = this->_header.find("\r\n", headerStart);
    if (headerEnd == std::string::npos)
        return "";

    return this->_header.substr(headerStart, headerEnd - headerStart);
}
