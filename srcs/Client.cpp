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

        this->_request.readRequest(this->_fd);
        if (!this->_request.ready())
            return;

        if (this->_request.empty()) {
            WebServ::removeIndex(index);
            return;
        }

        // std::cout << "===== FULL BODY =====" << std::endl;
        // std::cout << this->_request.getBody() << std::endl;
        // std::cout << "====================" << std::endl;

        std::string method = this->_request.getMethod();

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

    this->_request.clear();

    // std::cout << "===== RESPONSE =====" << std::endl;
    // std::cout << this->_response << std::endl;
    // std::cout << "====================" << std::endl;

    if (WebServ::pollFds[index].revents & POLLOUT) {
        size_t bytes = write(this->_fd, this->_response.c_str(), this->_response.length());
        if (bytes == (size_t)-1)
            throw Error("write");
    }
}

void Client::handleDirectory(const std::string& uri) {
    if (uri[uri.length() - 1] != '/') {
        this->_response = HttpResponse::redirectResponse(this->_request.getUri() + "/");
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

    responseBody += "<a href=\"../\">../</a>\r\n";
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

bool Client::fileSearch(std::string uri) {
    std::string path = uri.substr(0, uri.find("?"));
    std::string query = uri.substr(uri.find("?") + 1, uri.length());

    if (path == "/cgi-bin/search.py") {
        Cgi cgi("cgi-bin/search.py");
        cgi.setEnv("REQUEST_METHOD=GET");
        cgi.setEnv("QUERY_STRING=" + query);

        this->_response = cgi.getResponse();
        if (this->_response == "File not found\n")
            this->_response = HttpResponse::pageResponse(400, "default_pages/400.html");
        return 1;
    }
    return 0;
}

void Client::getMethod(void) {
    if (this->fileSearch(this->_request.getUri()))
        return;

    const std::string* redirect = this->_server->getRedirect(this->_request.getUri());

    if (redirect) {
        this->_response = HttpResponse::redirectResponse(*redirect);
        return;
    }

    std::string uri = this->_server->getRoot() + this->_request.getUri();

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
    Cgi cgi("cgi-bin/upload.py");

    cgi.setBody(this->_request.getBody());
    cgi.setEnv("REQUEST_METHOD=POST");
    cgi.setEnv("TRANSFER_ENCODING=chunked");
    cgi.setEnv("CONTENT_TYPE=" + this->_request.getHeaderValue("Content-Type: "));
    cgi.setEnv("CONTENT_LENGTH=" + this->_request.getHeaderValue("Content-Length: "));

    this->_response = cgi.getResponse();
}

void Client::deleteMethod(void) {
    this->_response = HttpResponse::pageResponse(501, "default_pages/501.html");
}
