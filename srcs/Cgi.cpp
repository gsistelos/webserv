#include "Cgi.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Error.hpp"

#define BUFFER_SIZE 1024 * 1024  // 1 MB

Cgi::Cgi(const std::string& request, size_t headerEnd, std::string& response) : _request(request), _headerEnd(headerEnd), _response(response) {
    this->_argv.push_back(strdup("/usr/bin/python3"));
    this->_argv.push_back(NULL);
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

void Cgi::setCgiPath(const std::string& path) {
    this->_argv[1] = strdup(path.c_str());
}

void Cgi::pushEnv(const std::string& env) {
    this->_env.push_back(strdup(env.c_str()));
}

void Cgi::pushEnvFromHeader(const std::string& find, const std::string& set) {
    size_t startPos = this->_request.find(find.c_str());
    if (startPos > this->_headerEnd)
        throw Error("header content not found");

    startPos += find.length();

    size_t endPos = this->_request.find("\r\n", startPos);
    if (endPos == std::string::npos)
        throw Error("header content not found");

    this->pushEnv(set + this->_request.substr(startPos, endPos - startPos));
}

void Cgi::execScript(void) {
    this->_argv.push_back(NULL);
    this->_env.push_back(NULL);

    if (pipe(this->_requestFd) == -1)
        throw Error("pipe");

    // Send request to CGI

    ssize_t bytes = write(this->_requestFd[1], _request.c_str(), _request.length());
    close(this->_requestFd[1]);
    if (bytes == -1) {
        close(this->_requestFd[0]);
        throw Error("write");
    }

    if (pipe(this->_responseFd) == -1) {
        close(this->_requestFd[0]);
        throw Error("pipe");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(this->_requestFd[0]);
        close(this->_responseFd[0]);
        close(this->_responseFd[1]);
        throw Error("fork");
    }

    if (pid == 0) {
        close(this->_responseFd[0]);

        dup2(this->_requestFd[0], STDIN_FILENO);
        close(this->_requestFd[0]);

        dup2(this->_responseFd[1], STDOUT_FILENO);
        close(this->_responseFd[1]);

        execve(this->_argv[0], this->_argv.data(), this->_env.data());
        throw Error("execve");
    }

    close(this->_requestFd[0]);
    close(this->_responseFd[1]);

    waitpid(pid, NULL, 0);

    // Receive response from CGI

    char buffer[BUFFER_SIZE];

    bytes = read(this->_responseFd[0], buffer, BUFFER_SIZE);
    close(this->_responseFd[0]);
    if (bytes == -1)
        throw Error("read");

    this->_response.assign(buffer, bytes);
}
