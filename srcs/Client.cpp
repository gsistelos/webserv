#include "Client.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>

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

Client::~Client() {
    std::cout << "Client socketFd " << this->_socketFd << " closed" << std::endl;
    close(this->_socketFd);
}

int Client::getSocketFd(void) {
    return this->_socketFd;
}

const std::string& Client::getResponse(void) {
    return this->_response;
}

void Client::getMethod(void) {
    std::string page;

    std::istringstream headerStream(_header);
    headerStream >> page;

    if (page == "/")
        page = "./pages/index.html";
    else
        page = "./pages" + page;

    std::ifstream file(page.c_str());
    if (!file) {
        _response = "HTTP/1.1 404 Not Found\r\n\r\n";
        return;
    }

    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    file.close();

    std::string fileContent = contentStream.str();

    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 200 OK\r\n";
    responseStream << "Content-Type: text/html\r\n";
    responseStream << "Content-Length: " << fileContent.length() << "\r\n";
    responseStream << "\r\n";
    responseStream << fileContent;

    _response = responseStream.str();
}

void Client::postMethod(void) {
    std::ofstream outputFile("file.xml");
    outputFile.write(_content.c_str(), _content.length());
    outputFile.close();

    getMethod();
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}

void Client::request(const std::string& request) {
    size_t headerEnd;

    headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        this->_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        return;
    }

    this->_header = request.substr(0, headerEnd);
    this->_content = request.substr(headerEnd);

    std::string method;

    std::istringstream headerStream(_header);
    headerStream >> method;

    _header.erase(0, method.length());

    if (method == "GET")
        getMethod();
    else if (method == "POST")
        postMethod();
    else if (method == "DELETE")
        deleteMethod();
    else
        _response = "HTTP/1.1 400 Method Not Supported\r\n\r\n";
}
