#include "Cgi.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

// Utils

void toUpperCase(std::string& content) {
    for (unsigned int i = 0; i < content.length(); ++i) {
        content[i] = std::toupper(content[i]);
    }
}

Cgi::Cgi(void) {
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

// Setters

void Cgi::setEnv(std::string& request) {
    this->_request = request;
    this->_env.push_back(getEnvFromHeader("Content-Type"));
    this->_env.push_back(getEnvFromHeader("Content-Length"));
    this->_env.push_back(strdup("AUTH_TYPE=Basic"));
    this->_env.push_back(strdup("DOCUMENT_ROOT=./"));
    this->_env.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    this->_env.push_back(strdup("HTTP_COOKIE="));
    this->_env.push_back(strdup("PATH_INFO="));
    this->_env.push_back(strdup("PATH_TRANSLATED=.//"));
    this->_env.push_back(strdup("QUERY_STRING="));
    this->_env.push_back(strdup("REDIRECT_STATUS=200"));
    this->_env.push_back(strdup("REMOTE_ADDR=localhost:8002"));
    this->_env.push_back(strdup("REQUEST_METHOD=POST"));
    this->_env.push_back(strdup("REQUEST_URI=/cgi-bin/upload.py"));
    this->_env.push_back(strdup("SCRIPT_FILENAME=upload.py"));
    this->_env.push_back(strdup("SCRIPT_NAME=cgi-bin/upload.py"));
    this->_env.push_back(strdup("SERVER_NAME=localhost"));
    this->_env.push_back(strdup("SERVER_PORT=8080"));
    this->_env.push_back(strdup("SERVER_PROTOCOL=HTTP/1.1"));
    this->_env.push_back(strdup("SERVER_SOFTWARE=AMANIX"));
    this->_env.push_back(NULL);
}

void Cgi::setArgv(void) {
    this->_argv.push_back(strdup("cgi-bin/upload.py"));
    this->_argv.push_back(NULL);
}

// Getters

char** Cgi::getEnv(void) {
    return this->_env.data();
}

char** Cgi::getArgv(void) {
    return this->_argv.data();
}

char* Cgi::getEnvFromHeader(std::string name) {
    size_t startPos = _request.find(name);
    size_t separator = _request.find(":", startPos);
    size_t endPos = _request.find("\r\n", separator);
    toUpperCase(name);
    std::string env = name + "=" + _request.substr(separator + 2, endPos - separator - 2);
    env.replace(env.find("-"), 1, "_");
    return strdup(env.c_str());
}

// Methods

void Cgi::createResponse(std::string& clientResponse) {
    char buffer[30000];
    size_t bytesRead = read(this->_responseFd[0], buffer, 30000);
    this->_response.append("HTTP/1.1 200 OK\r\n");
    this->_response.append("Content-Type: text/html\r\n");
    // TODO: check if when u request a page, the content-length is the size of header + content or only content
    // and then, set the content-lenght below properly.
    std::ostringstream strBytesRead;
    strBytesRead << bytesRead;
    this->_response.append("Content-Length: " + strBytesRead.str() + "\r\n");
    this->_response.append("\r\n");
    this->_response.append(buffer);
    if (bytesRead <= 0) {
        // TODO: set response to 500 and throw error response
        return;
    }
    clientResponse = this->_response;
}

void Cgi::execScript(void) {
    if (pipe(this->_pipefd) == -1 || pipe(this->_responseFd) == -1) {
        std::cout << "Error: pipe creation failed" << std::endl;
        exit(1);
    }

    int pid = fork();
    if (pid == 0) {
        close(this->_pipefd[1]);
        dup2(this->_pipefd[0], STDIN_FILENO);
        dup2(this->_responseFd[1], STDOUT_FILENO);
        close(this->_pipefd[0]);
        if (execve("cgi-bin/upload.py", this->getArgv(), this->getEnv()) == -1) {
            std::cout << "Error: execve failed" << std::endl;
            exit(1);
        }
    } else {
        close(this->_pipefd[0]);
        write(this->_pipefd[1], _request.c_str(), _request.length());
        close(this->_pipefd[1]);
        waitpid(pid, NULL, 0);
    }
}
