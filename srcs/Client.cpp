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

    std::vector<char> buffer(bodySize + 1);

    size_t bytesRead = read(this->_fd, buffer.data(), bodySize);

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

    std::string request(buffer.data());
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        this->_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        return;
    }

    this->_header = request.substr(0, headerEnd);
    this->_body = request.substr(headerEnd + 4);

    std::string method;
    Parser::extractWord(this->_header, method);

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
    std::string filePath = ".";
    std::string headerPath;

    std::istringstream headerStream(_header);
    headerStream >> headerPath;
    std::string extension = headerPath.substr(headerPath.find_last_of(".") + 1);

    if (headerPath == "/")
        filePath.append("/pages/home/index.html");
    else if (headerPath == "/style.css")
        filePath.append("/pages/home/style.css");
    else
        filePath.append(headerPath);
    // TODO: i would like to get the css file with an relative path, instead of an absolute path, inside the html file
    // as the initial route is "/", when the css is called we wil receive the path as "/style.css" in the request
    // we can check this "/style.css" path and redirect to "/pages/home/style.css"
    // or we dont need to check the "/style.css" path and redirect but we must call the css with an absolute path in the html file
    // also, we can do a kinda of "route checker", and open the file that correspond to the route (e.g.: localhost:8080/index.html open pages/home/index.html)

    std::ifstream file(filePath.c_str());
    if (!file) {
        this->_response = "HTTP/1.1 404 Not Found\r\n\r\n";
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string fileContent = buffer.str();
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    if (extension == "css")
        response << "Content-Type: text/css\r\n";
    else
        response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << fileContent.length() << "\r\n";
    response << "\r\n";
    response << fileContent;

    this->_response = response.str();
}

void Client::postMethod(void) {
    Cgi uploadCgi(this->_header, this->_body, this->_response);

    uploadCgi.sendCgiBody();
    uploadCgi.execScript();
    uploadCgi.buildResponse();
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}
