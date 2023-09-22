#include "Cgi.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Error.hpp"

#define BUFFER_SIZE 1024 * 1024  // 1 MB

Cgi::Cgi(const std::string& path, const std::string& header, const std::string& body) : _header(header), _body(body) {
    this->_argv.push_back(strdup(path.c_str()));
    this->_argv.push_back(NULL);
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

void Cgi::setEnv(const std::string& env) {
    this->_env.push_back(strdup(env.c_str()));
}

void Cgi::setEnvFromHeader(const std::string& headerName, const std::string& envKey) {
    size_t startPos = this->_header.find(headerName.c_str());
    if (startPos > std::string::npos)
        throw Error("header content not found");

    startPos += headerName.length();

    size_t endPos = this->_header.find("\r\n", startPos);
    if (endPos == std::string::npos)
        throw Error("header content end not found");

    this->setEnv(envKey + this->_header.substr(startPos, endPos - startPos));
}

std::string Cgi::getResponse(void) {
    this->_argv.push_back(NULL);
    this->_env.push_back(NULL);

    int requestFd[2];
    int responseFd[2];

    if (pipe(requestFd) == -1)
        throw Error("pipe");

    if (pipe(responseFd) == -1) {
        close(requestFd[0]);
        close(requestFd[1]);
        throw Error("pipe");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(requestFd[0]);
        close(requestFd[1]);
        close(responseFd[0]);
        close(responseFd[1]);
        throw Error("fork");
    }

    if (pid == 0) {
        close(requestFd[1]);
        close(responseFd[0]);

        dup2(requestFd[0], STDIN_FILENO);
        close(requestFd[0]);

        dup2(responseFd[1], STDOUT_FILENO);
        close(responseFd[1]);

        execve(this->_argv[0], this->_argv.data(), this->_env.data());
        throw Error("execve");
    }

    close(requestFd[0]);
    close(responseFd[1]);

    // Send request to CGI

    ssize_t bytes = write(requestFd[1], this->_body.c_str(), this->_body.length());
    close(requestFd[1]);
    if (bytes == -1) {
        close(responseFd[0]);
        throw Error("write");
    }

    waitpid(pid, NULL, 0);

    // Receive response from CGI

    char buffer[BUFFER_SIZE];

    bytes = read(responseFd[0], buffer, BUFFER_SIZE);
    close(responseFd[0]);
    if (bytes == -1)
        throw Error("read");

    return std::string(buffer, bytes);
}
