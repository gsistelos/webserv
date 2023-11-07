#include "Client.hpp"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "ConfigBlock.hpp"
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
    std::string index;
    std::string path;
    bool hasAutoindex;
    bool isCgi;

    std::string server_name;
    try {
        server_name = this->_request.getHeaderValue("Host");
    } catch (const Error& e) {
        (void)e;
    }

    const ConfigBlock& config = this->_server.getConfig(server_name);

    if (this->_request.getContentLength() > config.getMaxBodySize()) {
        this->error(413, config);
        return 413;
    }

    const Location* location = config.getLocation(uri);
    if (location != NULL) {
        if (location->isMethodAllowed(this->_request.getMethod()) == false) {
            this->error(405, config);
            return 405;
        }

        const std::string* redirect = location->getRedirect();
        if (redirect != NULL) {
            this->_response.redirect(*redirect);
            return 301;
        }

        index = location->getIndex();

        const std::string* alias = location->getAlias();
        if (alias != NULL)
            path = *alias + uri.substr(location->getUri().length());
        else
            path = config.getRoot() + uri;

        hasAutoindex = location->getAutoindex();
        isCgi = location->isCgiExtension(path);
    } else {
        index = "index.html";
        path = config.getRoot() + uri;
        hasAutoindex = false;
        isCgi = false;
    }

    struct stat pathStat;

    if (stat(path.c_str(), &pathStat) != 0) {
        this->error(404, config);
        return 404;
    }

    if (S_ISDIR(pathStat.st_mode) == true) {
        if (path[path.length() - 1] != '/') {
            this->_response.redirect(uri + "/");
            return 301;
        }

        std::string newUri = uri + index;
        int status = this->parseRequest(newUri);
        if (status == 404) {
            if (hasAutoindex) {
                this->_response.directoryList(path);
                return 200;
            }
            this->error(403, config);
            return 403;
        }
        return status;
    }

    if (S_ISREG(pathStat.st_mode) == false) {
        this->error(403, config);
        return 403;
    }

    if (isCgi) {
        this->_response.cgi(path, this->_request);
        return 200;
    }

    this->_response.file(200, path);

    return 200;
}

void Client::error(int statusCode, const ConfigBlock& config) {
    const std::string* errorPage = config.getErrorPage(statusCode);
    if (errorPage == NULL) {
        this->_response.error(statusCode);
        return;
    }

    this->_response.redirect(*errorPage);
}
