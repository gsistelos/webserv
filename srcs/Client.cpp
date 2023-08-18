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

#include "Error.hpp"
#include "Server.hpp"

Client::Client(Server* server) {
    this->_server = server;

    this->_addrlen = sizeof(this->_address);

    this->_socketFd = accept(server->getSocketFd(), (sockaddr*)&this->_address, &this->_addrlen);
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

Server* Client::getServer(void) {
    return this->_server;
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
    std::cout << "Start POST request" << std::endl;
    std::cout << _request << std::endl;
    std::cout << "End POST request" << std::endl;

    size_t contentTypeStart = _request.find("Content-Type: ") + 14;
    size_t contentTypeEnd = _request.find("\r\n", contentTypeStart);
    std::string contentType = "CONTENT_TYPE=" + _request.substr(contentTypeStart, contentTypeEnd - contentTypeStart);

    std::vector<char*> argv;
    argv.push_back(strdup("cgi-bin/upload.py"));
    argv.push_back(NULL);

    std::vector<char*> env;
    env.push_back(strdup("AUTH_TYPE=Basic"));
    env.push_back(strdup("CONTENT_LENGTH=213"));
    env.push_back(strdup(contentType.c_str()));
    env.push_back(strdup("DOCUMENT_ROOT=./"));
    env.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    env.push_back(strdup("HTTP_COOKIE="));
    env.push_back(strdup("PATH_INFO="));
    env.push_back(strdup("PATH_TRANSLATED=.//"));
    env.push_back(strdup("QUERY_STRING="));
    env.push_back(strdup("REDIRECT_STATUS=200"));
    env.push_back(strdup("REMOTE_ADDR=localhost:8002"));
    env.push_back(strdup("REQUEST_METHOD=POST"));
    env.push_back(strdup("REQUEST_URI=/cgi-bin/upload.py"));
    env.push_back(strdup("SCRIPT_FILENAME=upload.py"));
    env.push_back(strdup("SCRIPT_NAME=cgi-bin/upload.py"));
    env.push_back(strdup("SERVER_NAME=localhost"));
    env.push_back(strdup("SERVER_PORT=8080"));
    env.push_back(strdup("SERVER_PROTOCOL=HTTP/1.1"));
    env.push_back(strdup("SERVER_SOFTWARE=AMANIX"));
    env.push_back(NULL);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cout << "Error: pipe creation failed" << std::endl;
        exit(1);
    }

    int pid = fork();
    if (pid == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        if (execve("cgi-bin/upload.py", argv.data(), env.data()) == -1) {
            std::cout << "Error: execve failed" << std::endl;
            for (std::vector<char*>::iterator it = argv.begin(); it != argv.end(); ++it)
                free(*it);
            for (std::vector<char*>::iterator it = env.begin(); it != env.end(); ++it)
                free(*it);
            exit(1);
        }
    } else {
        close(pipefd[0]);
        write(pipefd[1], _request.c_str(), _request.length());
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
        for (std::vector<char*>::iterator it = argv.begin(); it != argv.end(); ++it)
            free(*it);
        for (std::vector<char*>::iterator it = env.begin(); it != env.end(); ++it)
            free(*it);
    }
}

void Client::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}

void Client::request(const std::string& request) {
    size_t headerEnd;

    _request = request;
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
