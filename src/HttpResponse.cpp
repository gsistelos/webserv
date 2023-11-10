#include "HttpResponse.hpp"

#include <dirent.h>

#include <iostream>
#include <map>
#include <sstream>

#include "Error.hpp"
#include "Parser.hpp"

HttpResponse::HttpResponse(void) {
}

HttpResponse::~HttpResponse() {
}

bool HttpResponse::ready(void) const {
    return this->_response.length() != 0;
}

const char* HttpResponse::c_str(void) const {
    return this->_response.c_str();
}

size_t HttpResponse::length(void) const {
    return this->_response.length();
}

void HttpResponse::clear(void) {
    this->_response.clear();
}

void HttpResponse::internalServerError(void) {
    this->_response =
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 306\r\n"
        "\r\n"
        "<html>\r\n"
        "    <head>\r\n"
        "        <title>500 Internal Server Error</title>\r\n"
        "    </head>\r\n"
        "    <body>\r\n"
        "        <h1>500 Internal Server Error</h1>\r\n"
        "        <p>The server encountered an unexpected condition that prevented it from fulfilling the request</p>\r\n"
        "        <a href=\"/\">Back to Home</a>\r\n"
        "    </body>\r\n"
        "</html>";
}

bool HttpResponse::empty(void) const {
    return this->_response.empty();
}

void HttpResponse::append(char* buffer, size_t bytes) {
    this->_response.append(buffer, bytes);
}

void HttpResponse::body(int statusCode, const std::string& contentType, const std::string& body) {
    try {
        std::stringstream ss;
        ss << "HTTP/1.1 " << statusCode << " " << HttpResponse::getStatusMessage(statusCode) << "\r\n"
           << "Content-Type: " << contentType << "\r\n"
           << "Content-Length: " << body.length() << "\r\n"
           << "\r\n"
           << body;

        this->_response = ss.str();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        this->internalServerError();
    }
}

void HttpResponse::file(int statusCode, const std::string& path) {
    try {
        std::string body;
        Parser::readFile(path, body);

        this->body(statusCode, this->contentType(path), body);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        this->internalServerError();
    }
}

void HttpResponse::redirect(const std::string& redirect) {
    try {
        std::stringstream ss;
        ss << "HTTP/1.1 301 Moved Permanently\r\n"
           << "Content-Length: 0\r\n"
           << "Location: " << redirect << "\r\n"
           << "\r\n";

        this->_response = ss.str();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        this->internalServerError();
    }
}

void HttpResponse::directoryList(const std::string& path) {
    try {
        std::stringstream body;
        body << "<html>\r\n"
             << "<head><title>Index of " << path << "</title></head>\r\n"
             << "<body>\r\n"
             << "<h1>Index of " << path << "</h1>\r\n"
             << "<hr>\r\n"
             << "<pre>\r\n"
             << "<a href=\"../\">../</a>\r\n";

        DIR* dir = opendir(path.c_str());
        if (dir == NULL)
            throw Error("opendir");

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;

            std::string element = entry->d_name;

            body << "<a href=\"" << element << "\">" << element << "</a>\r\n";
        }

        body << "</pre>\r\n"
             << "</hr>\r\n"
             << "</body>\r\n"
             << "</html>\r\n";

        if (closedir(dir) == -1)
            throw Error("closedir");

        this->body(200, "text/html", body.str());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        this->internalServerError();
    }
}

void HttpResponse::error(int statusCode) {
    try {
        std::stringstream defaultPage;
        defaultPage << "src/default_pages/" << statusCode << ".html";

        this->file(statusCode, defaultPage.str());
    } catch (const std::exception& e) {
        this->internalServerError();
    }
}

void HttpResponse::cgi(const std::string& path, const HttpRequest& request) {
    try {
        Cgi* cgi = new Cgi(*this);
        cgi->setEnv("REQUEST_METHOD=" + request.getMethod());
        if (request.getBody() != "") {
            cgi->setEnv("CONTENT_TYPE=" + request.getHeaderValue("Content-Type"));
            cgi->setEnv("CONTENT_LENGTH=" + request.getHeaderValue("Content-Length"));
        }
        cgi->setEnv("QUERY_STRING=" + request.getQuery());
        cgi->exec(path, request.getBody());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        this->internalServerError();
    }
}

std::string HttpResponse::contentType(const std::string& filename) {
    size_t start = filename.find_last_of('.');
    if (start == std::string::npos)
        return "text/plain";

    std::string extension = filename.substr(++start);

    return getMimeType(extension);
}

const std::string& HttpResponse::getStatusMessage(int statusCode) {
    static std::map<int, std::string> statusMessage;

    if (statusMessage.empty()) {
        statusMessage[200] = "OK";
        statusMessage[204] = "No Content";
        statusMessage[301] = "Moved Permanently";
        statusMessage[400] = "Bad Request";
        statusMessage[404] = "Not Found";
        statusMessage[403] = "Forbidden";
        statusMessage[405] = "Method Not Allowed";
        statusMessage[413] = "Request Entity Too Large";
        statusMessage[500] = "Internal Server Error";
        statusMessage[501] = "Not Implemented";
        statusMessage[502] = "Bad Gateway";
        statusMessage[505] = "HTTP Version Not Supported";
    }

    if (statusMessage.count(statusCode))
        return statusMessage.at(statusCode);
    throw Error("invalid status code");
}

const std::string& HttpResponse::getMimeType(const std::string& extension) {
    static std::map<std::string, std::string> mimeType;

    if (mimeType.empty()) {
        mimeType["html"] = "text/html";
        mimeType["css"] = "text/css";
        mimeType["js"] = "text/javascript";
        mimeType["jpg"] = "image/jpeg";
        mimeType["jpeg"] = "image/jpeg";
        mimeType["png"] = "image/png";
        mimeType["gif"] = "image/gif";
        mimeType["ico"] = "image/x-icon";
        mimeType["mp4"] = "video/mp4";
        mimeType["mp3"] = "audio/mpeg";
        mimeType["json"] = "application/json";
        mimeType["pdf"] = "application/pdf";
        mimeType["zip"] = "application/zip";
        mimeType["xml"] = "application/xml";
        mimeType["txt"] = "text/plain";
    }

    if (mimeType.count(extension))
        return mimeType.at(extension);
    return mimeType.at("txt");
}
