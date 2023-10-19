#include "Client.hpp"

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

    WebServ::push_back(this);
}

Client::~Client() {
}

void Client::handlePollin(int index) {
    try {
        std::cout << "Incoming data from client fd: " << this->_fd << std::endl;

        this->_request.readRequest(this->_fd);

        // std::cout << "BODY BEFORE" << std::endl;
        // std::cout << "===========================================" << std::endl;
        // std::cout << this->_request.getBody() << std::endl;
        // std::cout << "===========================================" << std::endl;

        if (!this->_request.ready()) {
            return;
        }

        // std::cout << "BODY AFTER" << std::endl;
        // std::cout << "===========================================" << std::endl;
        // std::cout << this->_request.getBody() << std::endl;
        // std::cout << "===========================================" << std::endl;

        if (this->_request.empty()) {
            WebServ::erase(index);
            return;
        }

        std::string method = this->_request.getMethod();
        if (!isValidRequest(method)) {
            this->_request.clear();
            return;
        }

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
}

bool Client::isValidRequest(const std::string& method) {
    std::string uri = this->_request.getUri();

    this->_request.setFile(this->_server->getRoot(), this->_request.getUri());
    std::string file = this->_request.getFile();
    std::string cgi_file = this->_request.getCgiFile();

    if ((stat(file.c_str(), &this->_request.getFileStat()) == -1) && cgi_file.empty() && uri != "/redirect/") {
        if (errno == ENOENT) {
            this->_response = HttpResponse::pageResponse(404, "default_pages/404.html");
        } else {
            this->_response = HttpResponse::internalServerError;
        }
        return false;
    }

    std::string location;
    struct stat fileStat = this->_request.getFileStat();
    if (S_ISDIR(fileStat.st_mode)) {
        if (uri[uri.length() - 1] != '/') {
            location = uri + "/";
        } else {
            location = uri;
        }
    } else if (S_ISREG(fileStat.st_mode)) {
        location = uri.substr(0, uri.find_last_of("/") + 1);
    }
    if (!this->_server->isAllowedMethod(location, method)) {
        this->_response = HttpResponse::pageResponse(405, "default_pages/405.html");
        return false;
    }
    return true;
}

void Client::handlePollout(void) {
    if (!this->_response.empty()) {
        size_t bytes = write(this->_fd, this->_response.c_str(), this->_response.length());
        if (bytes == (size_t)-1)
            throw Error("write");
        this->_response.clear();
    }
}

void Client::handleDirectory(const std::string& directory) {
    std::string file = this->_request.getFile();
    if (directory[directory.length() - 1] != '/') {
        this->_response = HttpResponse::redirectResponse(this->_request.getUri() + "/");
        return;
    }

    std::string index = file + "index.html";

    if (!access(index.c_str(), F_OK)) {
        this->_response = HttpResponse::pageResponse(200, index);
        return;
    }

    if (this->_server->getAutoIndex(this->_request.getUri())) {
        this->getDirectoryPage(file);

    } else {
        this->_response = HttpResponse::pageResponse(403, "default_pages/403.html");
    }
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

void Client::getMethod(void) {
    std::string file = this->_request.getFile();

    if (this->_request.getCgiFile() == "register") {
        Cgi* cgi = new Cgi(file, this->_request.getBody(), this->_response);
        cgi->setEnv("REQUEST_METHOD=GET");
        cgi->setEnv("QUERY_STRING=" + this->_request.getQuery());
        cgi->execScript();
        return;
    } else if (this->_request.getCgiFile() == "upload") {
        this->_response = HttpResponse::pageResponse(204, "default_pages/204.html");
        return;
    }

    std::string uri = this->_request.getUri();
    const std::string* redirect = this->_server->getRedirect(uri);

    if (redirect != NULL) {
        this->_response = HttpResponse::redirectResponse(*redirect);
        return;
    }

    struct stat fileStat = this->_request.getFileStat();
    if (S_ISDIR(fileStat.st_mode)) {
        this->handleDirectory(uri);

    } else if (S_ISREG(fileStat.st_mode)) {
        this->_response = HttpResponse::pageResponse(200, file);
    }
}

void Client::postMethod(void) {
    std::string file = this->_request.getFile();
    if (this->_request.getCgiFile() != "upload") {
        this->_response = HttpResponse::pageResponse(204, "default_pages/204.html");
        return;
    }
    Cgi* cgi = new Cgi(file, this->_request.getBody(), this->_response);
    cgi->setEnv("REQUEST_METHOD=POST");
    cgi->setEnv("CONTENT_TYPE=" + this->_request.getHeaderValue("Content-Type: "));
    cgi->setEnv("CONTENT_LENGTH=" + this->_request.getHeaderValue("Content-Length: "));
    cgi->execScript();
}

void Client::deleteMethod(void) {
    this->_response = HttpResponse::pageResponse(501, "default_pages/501.html");
}
