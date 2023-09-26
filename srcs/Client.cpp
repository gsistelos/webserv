#include "Client.hpp"

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>

#include "Cgi.hpp"
#include "Error.hpp"
#include "HttpResponse.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

#define BUFFER_SIZE 2048  // 2 KB

Client::Client(Server* server) {
    this->_server = server;

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    this->_fd = accept(server->getFd(), (sockaddr*)&address, &addrlen);
    if (this->_fd == -1)
        throw Error("accept");

    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC))
        throw Error("fcntl");

    WebServ::sockets.push_back(this);
    WebServ::pushPollfd(this->_fd);

    // Get client address and port

    char clientIp[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &address.sin_addr, clientIp, INET_ADDRSTRLEN) == NULL)
        throw Error("inet_ntop");

    int clientPort = ntohs(address.sin_port);

    std::cout << "Accepted client: " << clientIp << ":" << clientPort << " on fd " << this->_fd << std::endl;
}

Client::~Client() {
}

void Client::handlePollin(int index) {
    try {
        std::cout << "Incoming data from client fd: " << this->_fd << std::endl;

        this->readHeader(index);

        this->readBody();

        std::string method;
        Parser::getWord(this->_header, method);

        if (method == "GET")
            getMethod();
        else if (method == "POST")
            postMethod();
        else if (method == "DELETE")
            deleteMethod();
        else
            this->_response = HttpResponse::pageResponse(405, "default_pages/405.html");
    } catch (const Error& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        this->_response = HttpResponse::internalServerError;
    }

    if (WebServ::pollFds[index].revents & POLLOUT) {
        size_t bytes = write(this->_fd, this->_response.c_str(), this->_response.length());
        if (bytes == (size_t)-1)
            throw Error("write");
    }
}

void Client::readHeader(int index) {
    char headerBuffer[BUFFER_SIZE];

    size_t bytes = read(this->_fd, headerBuffer, BUFFER_SIZE);
    if (bytes == (size_t)-1) {
        this->_response = HttpResponse::pageResponse(400, "default_pages/400.html");
        throw Error("read");
    }
    if (bytes == 0) {
        WebServ::removeIndex(index);
        return;
    }

    this->_header.assign(headerBuffer, bytes);

    // Split header and body

    size_t headerEnd = this->_header.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        this->_body.clear();
    } else {
        headerEnd += 4;
        this->_body = this->_header.substr(headerEnd);
        this->_header = this->_header.substr(0, headerEnd);
    }

    // std::cout << "======= HEADER =======" << std::endl;
    // std::cout << this->_header << std::endl;
    // std::cout << "======================" << std::endl;
}

void Client::readBody(void) {
    if (!this->_body.length())
        return;

    std::string contentLengthHeader;
    try {
        contentLengthHeader = this->getHeaderValue("Content-Length: ");
    } catch (const Error& e) {
        this->_response = HttpResponse::pageResponse(411, "default_pages/411.html");
        throw Error("411 Length Required");
    }

    size_t contentLength = std::strtoll(contentLengthHeader.c_str(), NULL, 10);
    if (errno == ERANGE) {
        this->_response = HttpResponse::pageResponse(400, "default_pages/400.html");
        throw Error("400 Bad Request");
    }

    if (contentLength > this->_server->getMaxBodySize()) {
        this->_response = HttpResponse::pageResponse(413, "default_pages/413.html");
        throw Error("413 Payload Too Large");
    }

    contentLength -= this->_body.length();

    char* bodyBuffer = new char[contentLength];

    size_t bytes;
    size_t bytesRead = 0;
    while (bytesRead < contentLength) {
        if (BUFFER_SIZE < contentLength - bytesRead)
            bytes = read(this->_fd, bodyBuffer + bytesRead, BUFFER_SIZE);
        else
            bytes = read(this->_fd, bodyBuffer + bytesRead, contentLength - bytesRead);
        if (bytes == (size_t)-1) {
            this->_response = HttpResponse::pageResponse(400, "default_pages/400.html");
            throw Error("read");
        }
        bytesRead += bytes;
    }

    this->_body.insert(this->_body.length(), bodyBuffer, bytesRead);
    delete[] bodyBuffer;

    // std::cout << "======= BODY =======" << std::endl;
    // std::cout << this->_body << std::endl;
    // std::cout << "====================" << std::endl;
}

void Client::handleDirectory(const std::string& uri) {
    if (uri[uri.length() - 1] != '/') {
        this->_response = HttpResponse::redirectResponse(uri.substr(this->_server->getRoot().length()) + "/");
        return;
    }

    std::string index = uri + "index.html";

    if (!access(index.c_str(), F_OK)) {
        this->_response = HttpResponse::pageResponse(200, index);
        return;
    }

    if (this->_server->getAutoindex())
        this->getDirectoryPage(uri);
    else
        this->_response = HttpResponse::pageResponse(403, "default_pages/403.html");
}

void Client::getDirectoryPage(const std::string& uri) {
    std::string responseBody =
        "<html>\r\n"
        "<head><title>Index of " +
        uri +
        "</title></head>\r\n"
        "<body>\r\n"
        "<h1>Index of " +
        uri +
        "</h1>\r\n"
        "<hr>\r\n"
        "<pre>\r\n";

    DIR* dir = opendir(uri.c_str());
    if (dir == NULL)
        throw Error("opendir");

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        std::string path = entry->d_name;

        responseBody += "<a href=\"" + path + "\">" + path + "</a>\r\n";
    }

    responseBody +=
        "</pre>\r\n"
        "</hr>\r\n"
        "</body>\r\n"
        "</html>\r\n";

    if (closedir(dir) == -1)
        throw Error("closedir");

    HttpResponse response;
    response.setStatusCode(200);
    response.setHeader("Content-Type", "text/html");
    response.setHeader("Content-Length", responseBody.length());
    response.setBody(responseBody);

    this->_response = response.toString();
}

// HTTP methods

void Client::getMethod(void) {
    std::string uri;
    Parser::getWord(this->_header, uri, 3);

    const std::string* redirect = this->_server->getRedirect(uri);

    if (redirect) {
        this->_response = HttpResponse::redirectResponse(*redirect);
        return;
    }

    uri = this->_server->getRoot() + uri;

    struct stat uriStat;
    if (stat(uri.c_str(), &uriStat)) {
        if (errno == ENOENT)
            this->_response = HttpResponse::pageResponse(404, "default_pages/404.html");
        else
            this->_response = HttpResponse::internalServerError;
        return;
    }

    if (S_ISDIR(uriStat.st_mode))
        this->handleDirectory(uri);
    else if (S_ISREG(uriStat.st_mode))
        this->_response = HttpResponse::pageResponse(200, uri);
    else
        this->_response = HttpResponse::pageResponse(400, "default_pages/400.html");
}

void Client::postMethod(void) {
    Cgi cgi("cgi-bin/upload.py", this->_body);

    cgi.setEnv("REQUEST_METHOD=POST");
    cgi.setEnv("TRANSFER_ENCODING=chunked");
    cgi.setEnv("CONTENT_TYPE=" + this->getHeaderValue("Content-Type: "));
    cgi.setEnv("CONTENT_LENGTH=" + this->getHeaderValue("Content-Length: "));

    this->_response = cgi.getResponse();
}

void Client::deleteMethod(void) {
    this->_response = HttpResponse::pageResponse(501, "default_pages/501.html");
}

// Private getters

std::string Client::getHeaderValue(const std::string& header) {
    size_t headerStart = this->_header.find(header);
    if (headerStart == std::string::npos)
        throw Error("header not found");

    headerStart += header.length();
    size_t headerEnd = this->_header.find("\r\n", headerStart);
    if (headerEnd == std::string::npos)
        throw Error("header not found");

    return this->_header.substr(headerStart, headerEnd - headerStart);
}

// Static map getters
