#include "Response.hpp"

#include <fstream>
#include <iostream>  // DEBUG
#include <sstream>

Response::Response(void) : _request(""), _response("") {
}

Response::Response(const std::string &request)
    : _request(request), _response("") {
    std::string method;

    std::istringstream requestStream(_request);
    requestStream >> method;

    std::cout << "Method: " << method << std::endl;  // DEBUG

    _request.erase(0, method.length());

    if (method == "GET")
        getMethod();
    else if (method == "POST")
        postMethod();
    else if (method == "DELETE")
        deleteMethod();
    else
        _response = "HTTP/1.1 400 Method Not Supported\n\n";
}

Response::Response(const Response &other)
    : _request(other._request), _response(other._response) {
}

Response::~Response() {
}

Response &Response::operator=(const Response &other) {
    _response = other._response;

    return *this;
}

const std::string &Response::getResponse(void) {
    return _response;
}

void Response::getMethod(void) {
    std::string page;

    std::istringstream requestStream(_request);
    requestStream >> page;

    std::cout << "Page: " << page << std::endl;  // DEBUG

    if (page == "/")
        page = "./pages/index.html";
    else
        page = "./pages" + page;

    std::ifstream file(page.c_str());
    if (!file) {
        _response = "HTTP/1.1 404 Not Found\n\n";
        return;
    }

    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    file.close();

    std::string fileContent = contentStream.str();

    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 200 OK\n";
    responseStream << "Content-Type: text/html\n";
    responseStream << "Content-Length: " << fileContent.length() << "\n";
    responseStream << "\n";
    responseStream << fileContent;

    _response = responseStream.str();
}

void Response::postMethod(void) {
    _response = "HTTP/1.1 400 Method In Development";
}

void Response::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development";
}
