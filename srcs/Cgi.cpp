#include "Cgi.hpp"

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

#define BUFFER_SIZE 30000

Cgi::Cgi(const std::string& request, std::string& response) : _request(request), _response(response) {
    this->_argv.push_back(strdup("/usr/bin/python3"));

    if (pipe(this->_pipefd) == -1 || pipe(this->_responseFd) == -1) {
        std::cout << "webserv: pipe: " << strerror(errno) << std::endl;
        return;
    }

    write(this->_pipefd[1], this->_request.c_str(), this->_request.length());
    close(this->_pipefd[1]);
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

void Cgi::pushArgv(const std::string& argv) {
    this->_argv.push_back(strdup(argv.c_str()));
}

void Cgi::pushEnv(const std::string& env) {
    this->_env.push_back(strdup(env.c_str()));
}

void Cgi::execScript(void) {
    this->_argv.push_back(NULL);
    this->_env.push_back(NULL);

    int pid = fork();

    if (pid == 0) {
        close(this->_responseFd[0]);

        dup2(this->_pipefd[0], STDIN_FILENO);
        close(this->_pipefd[0]);

        dup2(this->_responseFd[1], STDOUT_FILENO);
        close(this->_responseFd[1]);

        execve(this->_argv[0], this->_argv.data(), this->_env.data());
        std::cout << "webserv: execve: " << strerror(errno) << std::endl;
        return;
    } else {
        close(this->_responseFd[1]);
        close(this->_pipefd[0]);
        waitpid(pid, NULL, 0);
    }
}

void Cgi::buildResponse(void) {
    this->_response.clear();

    char buffer[BUFFER_SIZE + 1];

    size_t bytesRead = read(this->_responseFd[0], buffer, BUFFER_SIZE);
    close(this->_responseFd[0]);

    std::ostringstream strBytesRead;
    strBytesRead << bytesRead;
    if (bytesRead <= 0) {
        this->_response.append("HTTP/1.1 500 Internal Server Error\r\n");
        this->_response.append("Content-Type: text/html\r\n");
        this->_response.append("\r\n");
        this->_response.append("<html>");
        this->_response.append("<p> ERROR: CGI Response is empty. </p>");
        this->_response.append("<html>");
    } else {
        this->_response.append(buffer);
    }
}
