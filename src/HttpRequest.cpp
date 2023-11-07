#include "HttpRequest.hpp"

#include <unistd.h>

#include <cstdlib>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

#define BUFFER_SIZE 2048  // 2KB

HttpRequest::HttpRequest(void) {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::ready(void) const {
    if (this->_isChunked)
        return this->_chunkSize == 0;
    else
        return this->_body.length() == this->_contentLength;
}

bool HttpRequest::empty(void) const {
    return this->_header.empty();
}

void HttpRequest::clear(void) {
    this->_header.clear();
}

const std::string& HttpRequest::getHeader(void) const {
    return this->_header;
}

const std::string& HttpRequest::getBody(void) const {
    return this->_body;
}

const std::string& HttpRequest::getQuery(void) const {
    return this->_query;
}

const std::string& HttpRequest::getMethod(void) const {
    return this->_method;
}

const std::string& HttpRequest::getUri(void) const {
    return this->_uri;
}

const size_t& HttpRequest::getContentLength(void) const {
    return this->_contentLength;
}

std::string HttpRequest::getHeaderValue(const std::string& key) const {
    size_t headerStart = this->_header.find(key);
    if (headerStart == std::string::npos)
        throw Error("Header not found");

    headerStart += key.length();
    size_t headerEnd = this->_header.find("\r\n", headerStart);
    if (headerEnd == std::string::npos)
        return this->_header.substr(headerStart);

    return this->_header.substr(headerStart, headerEnd - headerStart);
}

void HttpRequest::setUri(const std::string& uri) {
    this->_uri = uri;
}

void HttpRequest::readRequest(int fd) {
    if (this->_header.empty()) {
        readHeader(fd);
        return;
    }

    if (this->_isChunked)
        readChunkedBody(fd);
    else
        readContentLengthBody(fd);
}

void HttpRequest::readHeader(int fd) {
    std::cout << "Reading header from fd: " << fd << std::endl;

    char buffer[BUFFER_SIZE];

    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read == -1)
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

    this->_query = this->_uri.substr(this->_uri.find("?") + 1);
    this->_uri = this->_uri.substr(0, this->_uri.find("?"));

    std::cout << std::endl
              << "===== REQUEST =====" << std::endl
              << std::endl;
    std::cout << this->_method << " "
              << this->_uri;
    std::cout << this->_header << std::endl;
    std::cout << "===================" << std::endl
              << std::endl;

    size_t pos = this->_header.find("Transfer-Encoding: chunked");
    if (pos != std::string::npos) {
        this->_isChunked = true;
        this->_chunkSize = 1;
        return;
    }

    this->_isChunked = false;

    try {
        std::string contentLength = this->getHeaderValue("Content-Length: ");
        this->_contentLength = std::strtoll(contentLength.c_str(), NULL, 10);
    } catch (const Error& e) {
        this->_contentLength = 0;
    }
}

void HttpRequest::readChunkedBody(int fd) {
    std::cout << "Reading body from fd: " << fd << std::endl;

    char buffer[BUFFER_SIZE];

    ssize_t bytes = read(fd, buffer, BUFFER_SIZE);
    if (bytes == -1)
        throw Error("read");

    std::string chunk(buffer, bytes);

    size_t pos = chunk.find("\r\n");
    if (pos == std::string::npos)
        throw Error("Invalid chunk");

    std::string chunkSize = chunk.substr(0, pos);

    this->_chunkSize = std::strtoll(chunkSize.c_str(), NULL, 16);

    chunk.erase(0, pos + 2);

    while (chunk.length() != this->_chunkSize) {
        bytes = read(fd, buffer, BUFFER_SIZE);
        if (bytes == -1)
            throw Error("read");

        chunk.append(buffer, bytes);
    }

    this->_body.append(chunk);
}

void HttpRequest::readContentLengthBody(int fd) {
    std::cout << "Reading body from fd: " << fd << std::endl;

    size_t toRead = this->_contentLength - this->_body.length();

    char* buffer = new char[toRead];

    ssize_t bytes = read(fd, buffer, toRead);
    if (bytes == -1)
        throw Error("read");

    this->_body.append(buffer, bytes);

    delete[] buffer;
}
