#include "Cgi.hpp"

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Error.hpp"
#include "HttpResponse.hpp"
#include "WebServ.hpp"

#define BUFFER_SIZE 64 * 1024  // 64 KB

static void close_pipe(int fd[2]) {
    close(fd[0]);
    close(fd[1]);
}

Cgi::Cgi(HttpResponse& response) : _responseFd(-1), _pid(-1), _totalBytes(0), _response(response) {
}

Cgi::~Cgi() {
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);

    if (this->_responseFd != -1)
        close(this->_responseFd);

    if (this->_pid > 0)
        kill(this->_pid, SIGKILL);
}

void Cgi::setEnv(const std::string& env) {
    this->_env.push_back(strdup(env.c_str()));
}

void Cgi::exec(const std::string& path, const std::string& body) {
    this->_env.push_back(NULL);

    // Setup pipes

    int requestFd[2];

    if (pipe(requestFd) != 0)
        throw Error("pipe");

    int responseFd[2];

    if (pipe(responseFd) != 0) {
        close_pipe(requestFd);
        throw Error("pipe");
    }

    if (fcntl(requestFd[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC) != 0) {
        close_pipe(requestFd);
        close_pipe(responseFd);
        throw Error("fcntl");
    }

    if (fcntl(responseFd[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC) != 0) {
        close_pipe(requestFd);
        close_pipe(responseFd);
        throw Error("fcntl");
    }

    this->_pid = fork();

    if (this->_pid == -1) {
        close_pipe(requestFd);
        close_pipe(responseFd);
        throw Error("fork");
    }

    cgiProcess process;
    process.pid = this->_pid;
    process.start_time = time(NULL);
    WebServ::cgiProcesses.push_back(process);

    if (this->_pid == 0) {
        close(requestFd[1]);
        close(responseFd[0]);

        dup2(requestFd[0], STDIN_FILENO);
        close(requestFd[0]);

        dup2(responseFd[1], STDOUT_FILENO);
        close(responseFd[1]);

        char* argv[2];
        argv[0] = strdup(path.c_str());
        argv[1] = NULL;

        execve(argv[0], argv, this->_env.data());
        free(argv[0]);
        std::cerr << "webserv: execve: " << path << ": " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    close(requestFd[0]);
    close(responseFd[1]);

    this->_fd = requestFd[1];

    this->_responseFd = responseFd[0];
    this->_body = body;

    WebServ::push_back(this);
}

void Cgi::checkRunningProcesses(int index) {
    time_t current_time = time(NULL);

    for (std::vector<cgiProcess>::iterator it = WebServ::cgiProcesses.begin(); it != WebServ::cgiProcesses.end();) {
        time_t elapsed_time = current_time - it->start_time;

        if (elapsed_time > 2) {
            kill(it->pid, SIGKILL);
            WebServ::erase(index);
            it = WebServ::cgiProcesses.erase(it);
        } else {
            ++it;
        }
    }
}


void Cgi::routine(int index) {
    this->checkRunningProcesses(index);
    if (WebServ::pollfds[index].revents & POLLIN)
        this->handlePollin(index);
    if (WebServ::pollfds[index].revents & POLLOUT)
        this->handlePollout(index);
}

void Cgi::handlePollout(int index) {
    try {
        ssize_t bytes = write(this->_fd,
                              this->_body.c_str() + this->_totalBytes,
                              this->_body.length() - this->_totalBytes);
        if (bytes == -1)
            throw Error("write");

        if (bytes == 0) {
        }

        this->_totalBytes += bytes;

        if (this->_totalBytes != this->_body.length())
            return;

        close(this->_fd);
        this->_fd = this->_responseFd;
        WebServ::pollfds[index].fd = this->_responseFd;
    } catch (const std::exception& e) {
        WebServ::erase(index);
        throw e;
    }
}

void Cgi::handlePollin(int index) {
    try {
        char buffer[BUFFER_SIZE];

        ssize_t bytes = read(this->_fd, buffer, BUFFER_SIZE);
        if (bytes == -1)
            throw Error("read");

        if (bytes == 0) {
        }

        this->_response.append(buffer, bytes);
    } catch (const std::exception& e) {
        WebServ::erase(index);
        throw e;
    }
}
