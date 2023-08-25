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

    std::cout << "Accepted client: " << clientIp << ":" << clientPort << " on fd " << this->_fd << std::endl;
}

Client::~Client() {
}

void Client::handlePollin(int index) {
    std::cout << "Incoming data from client fd: " << this->_fd << std::endl;

    size_t bodySize = this->_server->getMaxBodySize();

    std::vector<char> buffer(bodySize + 1);

    size_t bytesRead = read(this->_fd, buffer.data(), bodySize);

    if (bytesRead == (size_t)-1) {
        std::cerr << "webserv: read: " << strerror(errno) << std::endl;
        WebServ::removeIndex(index);
    }
    if (bytesRead == 0) {
        WebServ::removeIndex(index);
        return;
    }

    buffer[bytesRead] = '\0';

    std::string request(buffer.data());

    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        this->_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        return;
    }

    this->_header = request.substr(0, headerEnd);
    this->_content = request.substr(headerEnd + 4);

    std::string method = Parser::extractWord(this->_header);

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
    std::string page;

    std::istringstream headerStream(_header);
    headerStream >> page;

    if (page == "/")
        page = "./pages/home/index.html";
    else
        page = "./pages/upload/index.html";

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
    Cgi uploadCgi(this->_header, this->_content, this->_response);

    uploadCgi.execScript();
    uploadCgi.createResponse();
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}
