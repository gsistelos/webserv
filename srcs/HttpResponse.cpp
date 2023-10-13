#include "HttpResponse.hpp"

#include <unistd.h>

#include <iostream>
#include <sstream>

#include "Error.hpp"
#include "Parser.hpp"

// Constructor and destructor

HttpResponse::HttpResponse(void) {
}

HttpResponse::~HttpResponse() {
}

// Static variables

const char HttpResponse::internalServerError[] =
    "HTTP/1.1 500 Internal Server Error\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 178\r\n"
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

// Setters

void HttpResponse::setStatusCode(int statusCode) {
    this->_statusCode = statusCode;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    this->_headers[key] = value;
}

void HttpResponse::setHeader(const std::string& key, size_t value) {
    std::stringstream ss;
    ss << value;

    this->_headers[key] = ss.str();
}

void HttpResponse::setBody(const std::string& body) {
    this->_body = body;
}

// toString

std::string HttpResponse::toString(void) const {
    std::stringstream ss;

    ss << "HTTP/1.1 " << this->_statusCode << " " << HttpResponse::getStatusMessage(this->_statusCode) << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it)
        ss << it->first << ": " << it->second << "\r\n";

    ss << "\r\n";

    if (this->_body.length())
        ss << this->_body;

    return ss.str();
}

// Static functions

std::string HttpResponse::pageResponse(int status, const std::string& uri) {
    std::string body;
    Parser::readFile(uri, body);

    HttpResponse response;
    response.setStatusCode(status);

    std::string contentType = HttpResponse::contentType(uri);
    response.setHeader("Content-Type", contentType);
    if (!contentType.compare(0, 6, "image/"))
        response.setHeader("Cache-Control", "max-age=3600");

    response.setHeader("Content-Length", body.length());
    response.setBody(body);

    return response.toString();
}

std::string HttpResponse::redirectResponse(const std::string& uri) {
    HttpResponse response;
    response.setStatusCode(301);
    response.setHeader("Content-Length", 0);
    response.setHeader("Location", uri);

    return response.toString();
}

std::string HttpResponse::contentType(const std::string& uri) {
    size_t start = uri.find_last_of('.');
    if (start == std::string::npos)
        return "text/plain";

    std::string extension = uri.substr(++start);

    return getMimeType(extension);
}

// Static map getters

const std::string& HttpResponse::getStatusMessage(int statusCode) {
    static std::map<int, std::string> statusMessage;

    if (statusMessage.empty()) {
        statusMessage[200] = "OK";
        statusMessage[301] = "Moved Permanently";
        statusMessage[400] = "Bad Request";
        statusMessage[404] = "Not Found";
        statusMessage[403] = "Forbidden";
        statusMessage[405] = "Method Not Allowed";
        statusMessage[500] = "Internal Server Error";
        statusMessage[501] = "Not Implemented";
        statusMessage[502] = "Bad Gateway";
    }

    if (!statusMessage.count(statusCode))
        throw Error("invalid status code");
    return statusMessage.at(statusCode);
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
