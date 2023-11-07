#include "HttpRequest.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

#define HEADER_BUFFER_SIZE 2048   // 2KB
#define BODY_BUFFER_SIZE 1048576  // 1MB

HttpRequest::HttpRequest(void) {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::ready(void) const {
    return this->_isReady;
}

bool HttpRequest::empty(void) const {
    return this->_method.empty();
}

void HttpRequest::clear(void) {
    this->_method.clear();
}

const std::string& HttpRequest::getMethod(void) const {
    return this->_method;
}

const std::string& HttpRequest::getUri(void) const {
    return this->_uri;
}

const std::string& HttpRequest::getQuery(void) const {
    return this->_query;
}

const std::string& HttpRequest::getBody(void) const {
    return this->_body;
}

size_t HttpRequest::getContentLength(void) const {
    return this->_contentLength;
}

std::string HttpRequest::getHeaderValue(const std::string& key) const {
    std::string headerKey = key + ": ";

    size_t offset = 0;

    while (true) {
        size_t lineEnd = this->_header.find("\r\n", offset);
        if (lineEnd == std::string::npos) {
            errno = EINVAL;
            return "";
        }

        // Check header key

        if (this->_header.compare(offset, headerKey.length(), headerKey) == 0) {
            size_t start = offset + headerKey.length();

            return this->_header.substr(start, lineEnd - start);
        }

        // Skip to next line

        offset = lineEnd + 2;
    }
}

void HttpRequest::readRequest(int fd) {
    if (this->_method.empty()) {
        readHeader(fd);
        return;
    }

    if (this->_isChunked)
        readChunkedBody(fd);
    else
        readContentLengthBody(fd);
}

const char* HttpRequest::VersionNotSupported::what(void) const throw() {
    return "Version Not Supported";
}

const char* HttpRequest::BadRequest::what(void) const throw() {
    return "Bad Request";
}

void HttpRequest::readHeader(int fd) {
    this->_isReady = false;

    char buffer[HEADER_BUFFER_SIZE];

    ssize_t bytes = read(fd, buffer, HEADER_BUFFER_SIZE);
    if (bytes == -1)
        throw Error("read");

    std::string request(buffer, bytes);
    if (request.empty()) {
        this->clear();
        this->_isReady = true;
        return;
    }

    // Get method, uri, query and http version

    Parser::extractWord(request, this->_method);
    if (this->_method.empty())
        throw BadRequest();

    Parser::extractWord(request, this->_uri);
    if (this->_uri.empty())
        throw BadRequest();

    size_t queryStart = this->_uri.find("?");
    if (queryStart == std::string::npos) {
        this->_query.clear();
    } else {
        this->_query = this->_uri.substr(queryStart + 1);
        this->_uri = this->_uri.substr(0, queryStart);
    }

    std::string httpVersion;
    Parser::extractWord(request, httpVersion);
    if (httpVersion != "HTTP/1.1")
        throw VersionNotSupported();

    // Split header and body

    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        throw BadRequest();

    this->_header = request.substr(0, headerEnd + 2);

    // Get content length or chunked

    errno = 0;

    std::string transferEncoding = this->getHeaderValue("Transfer-Encoding");
    if (errno == 0) {
        if (transferEncoding != "chunked")
            throw BadRequest();

        this->_isChunked = true;

        return;
    }

    errno = 0;
    this->_isChunked = false;

    std::string contentLength = this->getHeaderValue("Content-Length");
    if (errno == 0) {
        for (std::string::iterator it = contentLength.begin(); it != contentLength.end(); it++) {
            if (std::isdigit(*it) == false)
                throw BadRequest();
        }

        this->_contentLength = std::strtoll(contentLength.c_str(), NULL, 10);
        if (errno == ERANGE) {
            errno = 0;
            throw BadRequest();
        }

        // Set remainder body

        size_t bytes = request.length() - headerEnd - 4;
        if (bytes > this->_contentLength)
            bytes = this->_contentLength;

        this->_body = request.substr(headerEnd + 4, bytes);

        if (this->_body.length() == this->_contentLength)
            this->_isReady = true;

        return;
    }

    this->_isReady = true;
}

void HttpRequest::readChunkedBody(int fd) {
    char buffer[BODY_BUFFER_SIZE];

    size_t bytes = read(fd, buffer, BODY_BUFFER_SIZE);
    if (bytes == (size_t)-1)
        throw Error("read");

    if (this->_chunk.empty()) {
        // It's a new chunk, get the chunk size

        this->_chunk.assign(buffer, bytes);

        size_t pos = this->_chunk.find("\r\n");
        if (pos == std::string::npos)
            throw BadRequest();

        std::string chunkSize = this->_chunk.substr(0, pos);

        for (std::string::iterator it = chunkSize.begin(); it != chunkSize.end(); it++) {
            if (std::isxdigit(*it) == false)
                throw BadRequest();
        }

        long long size = std::strtoll(chunkSize.c_str(), NULL, 16);
        if (errno == ERANGE) {
            errno = 0;
            throw BadRequest();
        }

        this->_chunkSize = size;

        if (this->_chunkSize == 0) {
            this->_isReady = true;
            return;
        }

        // Remove the first line and what exceeds the chunk size

        if (this->_chunk.length() - pos - 2 > this->_chunkSize)
            this->_chunk = this->_chunk.substr(pos + 2, this->_chunkSize);
        else
            this->_chunk.erase(0, pos + 2);
    } else {
        // It's a chunk continuation, append to the chunk, not exceeding the chunk size

        size_t diff = this->_chunkSize - this->_chunk.length();
        if (bytes > diff)
            bytes = diff;

        this->_chunk.append(buffer, bytes);
    }

    // Check if the chunk is complete

    if (this->_chunk.length() == this->_chunkSize) {
        this->_body.append(this->_chunk);
        this->_chunk.clear();
    }
}

void HttpRequest::readContentLengthBody(int fd) {
    char buffer[BODY_BUFFER_SIZE];

    size_t bytes = read(fd, &buffer, BODY_BUFFER_SIZE);
    if (bytes == (size_t)-1)
        throw Error("read");

    // Append to the body, not exceeding the body length

    size_t diff = this->_contentLength - this->_body.length();
    if (bytes > diff)
        bytes = diff;

    this->_body.append(buffer, bytes);

    // Check if the body is complete

    if (this->_body.length() == this->_contentLength)
        this->_isReady = true;
}
