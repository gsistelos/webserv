#include "Cgi.hpp"

#include <unistd.h>

#include "HttpResponse.hpp"

#define BUFFER_SIZE 64 * 1024  // 64 KB

Cgi::Cgi(const std::string& path, const std::string& body, std::string& response) : _body(body), _response(response) {
    this->_argv.push_back(strdup(path.c_str()));
    this->_argv.push_back(NULL);

    WebServ::fds.push_back(this);
    this->startPipes();
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

void Cgi::startPipes(void) {
    if (pipe(this->_requestFd) == -1)
        throw Error("pipe");

    if (pipe(this->_responseFd) == -1) {
        close(this->_requestFd[0]);
        close(this->_requestFd[1]);
        throw Error("pipe");
    }

    if (fcntl(this->_responseFd[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) {
        close(this->_requestFd[0]);
        close(this->_requestFd[1]);
        close(this->_responseFd[0]);
        close(this->_responseFd[1]);
        throw Error("fcntl");
    }

    this->_fd = this->_responseFd[0];

    // Fd to be monitored by WebServ class when the cgi finishes
    WebServ::push_back(this);
}

void Cgi::execScript(void) {
    this->_env.push_back(NULL);

    this->_pid = fork();
    if (this->_pid == -1) {
        close(this->_requestFd[0]);
        close(this->_requestFd[1]);
        close(this->_responseFd[0]);
        close(this->_responseFd[1]);
        throw Error("fork");
    }

    if (this->_pid == 0) {
        close(this->_requestFd[1]);
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

    // Send request to CGI (POST method)

    if (!this->_body.empty())
        this->sendBody();
}

void Cgi::sendBody(void) {
    ssize_t bytes = write(this->_requestFd[1], this->_body.c_str(), this->_body.length());
    close(this->_requestFd[1]);
    if (bytes == -1) {
        close(this->_responseFd[0]);
        throw Error("write");
    }
}

void Cgi::handlePollout(void) {}

void Cgi::handlePollin(int index) {
    (void)index;
    char buffer[BUFFER_SIZE];

    ssize_t bytes = read(this->_responseFd[0], buffer, BUFFER_SIZE);
    if (bytes == -1)
        throw Error("read");

    int status;
    waitpid(this->_pid, &status, 0);
    if (WEXITSTATUS(status) != 0)
        this->_response = HttpResponse::pageResponse(502, "default_pages/502.html");

    WebServ::erase(index);
}