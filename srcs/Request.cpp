#include "Request.hpp"

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
Request::Request(void) {
}

Request::Request(const std::string &request) {
    size_t headerEnd;

    headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = request.find("\n\n");

    if (headerEnd == std::string::npos)
        this->_header = request.substr(0, headerEnd);
    else {
        this->_header = request.substr(0, headerEnd);
        this->_content = request.substr(headerEnd, request.length());
    }

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

Request::~Request() {
}

const std::string &Request::getResponse(void) {
    return _response;
}

void Request::getMethod(void) {
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

void Request::postMethod(void) {
    int pid = fork();
    std::string eae = "cgi-bin/upload.py";
    char **argv = (char **)malloc(sizeof(char *) * 2);
    argv[0] = strdup(eae.c_str());
    argv[1] = NULL;

    if (pid == 0) {
        if (execve("cgi-bin/upload.py", argv, NULL) == -1) {
            std::cout << "Error: execve failed" << std::endl;
            exit(1);
        }
    }
    // std::ofstream outputFile("file.xml");
    // outputFile.write(_content.c_str(), _content.length());
    // outputFile.close();

    // getMethod();
}

void Request::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}
