#include "Request.hpp"

#include <fstream>
#include <sstream>

Request::Request(void) {
}

Request::Request(const std::string &request) {
    size_t headerEnd = request.find("\r\n\r\n");

    this->_header = request.substr(0, headerEnd);
    this->_content = request.substr(headerEnd, request.length());

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

void Request::postMethod(void) {
    std::ofstream outputFile("file.xml");
    outputFile.write(_content.c_str(), _content.length());
    outputFile.close();

    getMethod();
}

void Request::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}