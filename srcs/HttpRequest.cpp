#include "HttpRequest.hpp"

#include <unistd.h>

#include <cstdlib>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"

#define BUFFER_SIZE 1024  // 2KB

HttpRequest::HttpRequest(void) : _chunked(false), _content_length(0) {
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::ready(void) {
    if (this->_chunked && this->_body.find("0\r\n\r\n") != std::string::npos) {
        this->unchunkBody();
        return true;
    }
    return this->_body.length() == this->_content_length;
}

bool HttpRequest::empty(void) {
    return this->_header.empty();
}

const std::string& HttpRequest::getHeader(void) const {
    return this->_header;
}

const std::string& HttpRequest::getBody(void) const {
    return this->_body;
}

const std::string& HttpRequest::getCgiFile(void) const {
    return this->_cgi_file;
}

struct stat& HttpRequest::getFileStat(void) {
    return this->_file_stat;
}

const std::string& HttpRequest::getMethod(void) const {
    return this->_method;
}

const std::string& HttpRequest::getQuery(void) const {
    return this->_query;
}

const std::string& HttpRequest::getFile(void) const {
    return this->_file;
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

void HttpRequest::setFile(const std::string& root, const std::string& file) {
    this->_file = root + file;

    if (this->_file == "./root/cgi-bin/form_upload.py") {
        this->_cgi_file = "upload";
    } else {
        std::string register_cgi_file = this->_file.substr(0, this->_file.find("?"));
        if (register_cgi_file == "./root/cgi-bin/register.py") {
            this->_cgi_file = "register";
            this->_query = this->_file.substr(this->_file.find("?") + 1, this->_file.length());
            this->_file = register_cgi_file;
        }
    }
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

    // std::cout << "HEADER" << std::endl;
    // std::cout << "===========================================" << std::endl;
    // std::cout << this->_method << " " << this->_uri;
    // std::cout << this->_header << std::endl;
    // std::cout << "===========================================" << std::endl;

    std::string transfer_encoding = this->getHeaderValue("Transfer-Encoding: ");
    if (!transfer_encoding.empty()) {
        this->_chunked = true;
        return;
    }
    this->_chunked = false;
    std::string content_length = this->getHeaderValue("Content-Length: ");
    if (!content_length.empty()) {
        this->_content_length = std::strtoll(content_length.c_str(), NULL, 10);
    } else
        this->_content_length = 0;
}

void HttpRequest::unchunkBody() {
    // TODO: o body aqui deve ser completo, caso o envio do body seja particionado em varias requests, precisa ser concatenado antes de entrar aqui.
    std::string newBody;
    size_t pos = 0;

    while (pos < this->_body.size()) {
        // Encontre a próxima linha (CRLF)
        size_t crlfPos = this->_body.find("\r\n", pos);

        if (crlfPos == std::string::npos) {
            break;
        }

        // Obtenha o tamanho do chunk
        std::string chunkSizeLine = this->_body.substr(pos, crlfPos - pos);

        // Transforma o tamanho do chunk de hexadecimal para decimal
        int chunkSize = std::stoi(chunkSizeLine, nullptr, 16);

        // Atualize a posição para apontar para o início dos dados do chunk, apos o CRLF
        pos = crlfPos + 2;

        // Verifique se há dados suficientes no restante da string
        if (pos + chunkSize > this->_body.size()) {
            break;
        }

        // Extraia os dados do chunk e acrescente ao corpo de descompactação
        std::string chunkData = this->_body.substr(pos, chunkSize);
        newBody.append(chunkData);

        // Atualize a posição para apontar para o próximo chunk
        pos += chunkSize + 2;  // Avança para além do CRLF no final do chunk
    }
    this->_body = newBody;
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
    this->_cgi_file.clear();
}
