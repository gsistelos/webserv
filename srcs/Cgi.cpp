#include "Cgi.hpp"

#include <sys/wait.h>

#include <cstdlib>
#include <cstring>

#include "Error.hpp"

#define BUFFER_SIZE 1024 * 1024  // 1 MB

Cgi::Cgi(const std::string& header, const std::string& body, std::string& response) : _header(header), _body(body), _response(response) {
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

void Cgi::pushEnvFromHeader(const std::string& headerName, const std::string& envKey) {
    size_t startPos = this->_header.find(headerName);
    if (startPos == std::string::npos)
        return;

    startPos = this->_header.find(": ", startPos);
    if (startPos == std::string::npos)
        return;

    size_t endPos = this->_header.find("\r\n", startPos);
    if (endPos == std::string::npos)
        return;

    this->pushEnv(envKey + "=" + this->_header.substr(startPos + 2, endPos - startPos - 2));
}

void Cgi::execScript(void) {
    this->_argv.push_back(NULL);
    this->_env.push_back(NULL);

    if (pipe(this->_requestFd) == -1)
        throw Error("pipe");

    // Send request to CGI

    ssize_t bytes = write(this->_requestFd[1], _body.c_str(), _body.length());
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
