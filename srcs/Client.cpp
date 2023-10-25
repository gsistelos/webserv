#include "Client.hpp"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "Error.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

Client::Client(Server& server) : _server(server) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    this->_fd = accept(server.getFd(), (sockaddr*)&address, &addrlen);
    if (this->_fd == -1)
        throw Error("accept");

    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) != 0)
        throw Error("fcntl");

    WebServ::push_back(this);
}

Client::~Client() {
}

void Client::handlePollin(int index) {
    this->_request.readRequest(this->_fd);

    if (this->_request.ready() == false)
        return;

    if (this->_request.empty() == true) {
        WebServ::erase(index);
        return;
    }

    // std::cout << "===== REQUEST =====" << std::endl;
    // std::cout << this->_request.getMethod() << " "
    //           << this->_request.getUri();
    // std::cout << this->_request.getHeader() << std::endl;
    // std::cout << this->_request.getBody() << std::endl;
    // std::cout << "===================" << std::endl;

    this->parseRequest(this->_request.getUri());
    this->_request.clear();
}

void Client::handlePollout(int index) {
    (void)index;

    if (!this->_response.ready())
        return;

    // std::cout << "===== RESPONSE =====" << std::endl;
    // std::cout << this->_response.c_str() << std::endl;
    // std::cout << "===================" << std::endl;

    ssize_t bytes = write(this->_fd, this->_response.c_str(), this->_response.length());
    if (bytes == -1)
        throw Error("write");

    this->_response.clear();
}

int Client::parseRequest(const std::string& uri) {
    const Location* location = this->_server.getLocation(uri);
    if (location != NULL) {
        if (location->isMethodAllowed(this->_request.getMethod()) == false) {
            this->error(405);
            return 405;
        }

        const std::string* redirect = location->getRedirect();
        if (redirect != NULL) {
            this->_response.redirect(*redirect);
            return 301;
        }
    }

    std::string path = this->_server.getRoot() + uri;

    struct stat pathStat;

    if (stat(path.c_str(), &pathStat) != 0) {
        this->error(404);
        return 404;
    }

    if (S_ISDIR(pathStat.st_mode) == true) {
        if (uri[uri.length() - 1] != '/') {
            this->_response.redirect(uri + "/");
            return 301;
        }

        std::string index = uri + (location == NULL ? "index.html" : location->getIndex());
        int status = this->parseRequest(index);
        if (status == 404) {
            if (location != NULL && location->getAutoindex() == true) {
                this->_response.directoryList(path);
                return 200;
            }
            this->error(403);
            return 403;
        }
        return status;
    }

    if (S_ISREG(pathStat.st_mode) == false) {
        this->error(403);
        return 403;
    }

    std::cout << path << std::endl;

    if (location && location->isCgiExtension(path) == true) {
        this->_response.cgi(path, this->_request);
        return 200;
    }

    this->_response.file(200, path);

    return 200;
}

void Client::error(int statusCode) {
    const std::string* errorPage = this->_server.getErrorPage(statusCode);
    if (errorPage == NULL) {
        this->_response.error(statusCode);
        return;
    }

    this->_response.redirect(*errorPage);
}
