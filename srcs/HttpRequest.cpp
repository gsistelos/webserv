#include "HttpRequest.hpp"

#include <unistd.h>

#include <cstdlib>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

#define BUFFER_SIZE 1024  // 2KB

HttpRequest::HttpRequest(void) : _content_length(0) {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::ready(void) {
    return this->_body.length() == this->_content_length;
}

bool HttpRequest::empty(void) {
    // std::cout << "===== HEADER_LEN =====" << std::endl;
    // std::cout << this->_header.length() << std::endl;
    // std::cout << this->_header << std::endl;
    // std::cout << "======================" << std::endl;
    return this->_header.empty();
}

const std::string& HttpRequest::getHeader(void) const {
    return this->_header;
}

const std::string& HttpRequest::getBody(void) const {
    return this->_body;
}

const std::string& HttpRequest::getMethod(void) const {
    return this->_method;
}

const std::string& HttpRequest::getUri(void) const {
    return this->_uri;
}

std::string HttpRequest::getHeaderValue(const std::string& key) const {
    size_t header_start = this->_header.find(key);
    if (header_start == std::string::npos)
        return "";

    header_start += key.length();
    size_t header_end = this->_header.find("\r\n", header_start);
    if (header_end == std::string::npos)
        return this->_header.substr(header_start);

    return this->_header.substr(header_start, header_end - header_start);
}

void HttpRequest::readRequest(int fd) {
    if (_header.empty())
        readHeader(fd);
    else
        readBody(fd);
}

void HttpRequest::readHeader(int fd) {
    char buffer[BUFFER_SIZE];

    size_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read == (size_t)-1)
        throw Error("read");

    this->_header.assign(buffer, bytes_read);

    size_t header_end = this->_header.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        this->_body.clear();
    } else {
        header_end += 4;
        this->_body = this->_header.substr(header_end);
        this->_header = this->_header.substr(0, header_end);
    }

    Parser::extractWord(this->_header, this->_method);
    Parser::extractWord(this->_header, this->_uri);

    // std::cout << "===== HEADERS =====" << std::endl;
    // std::cout << this->_method << " " << this->_uri;
    // std::cout << this->_header << std::endl;
    // std::cout << "====================" << std::endl;

    std::string content_length = this->getHeaderValue("Content-Length: ");
    if (content_length.empty()) {
        this->_content_length = 0;
        return;
    }

    this->_content_length = std::strtoll(content_length.c_str(), NULL, 10);
}

void HttpRequest::readBody(int fd) {
    size_t to_read = this->_content_length - this->_body.length();

    char* buffer = new char[to_read];

    size_t bytes_read = read(fd, buffer, to_read);
    if (bytes_read == (size_t)-1)
        throw Error("read");

    this->_body.append(buffer, bytes_read);
}

void HttpRequest::clear(void) {
    this->_header.clear();
}
