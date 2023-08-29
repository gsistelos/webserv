#include "Cgi.hpp"

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

Cgi::Cgi(const std::string& header, const std::string& content, std::string& _response) : _header(header), _body(content), _response(_response) {
    this->setEnv();
    this->setArgv();
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

// Methods
void Cgi::buildResponse(void) {
    char buffer[30000];
    size_t bytesRead = read(this->_responseFd[0], buffer, 30000);
    close(this->_responseFd[0]);
    std::ostringstream strBytesRead;
    strBytesRead << bytesRead;
    this->_response.clear();
    if (bytesRead <= 0) {
        this->_response.append("HTTP/1.1 500 Internal Server Error\r\n");
        this->_response.append("Content-Type: text/html\r\n");
        this->_response.append("\r\n");
        this->_response.append("<html>");
        this->_response.append("<p> ERROR: CGI Response is empty. </p>");
        this->_response.append("<html>");
    } else {
        this->_response.append("HTTP/1.1 200 OK\r\n");
        this->_response.append("Content-Type: text/html\r\n");
        this->_response.append("Content-Length: " + strBytesRead.str() + "\r\n");
        this->_response.append("\r\n");
        this->_response.append(buffer);
        std::cout << "RESPONSE: " << this->_response << std::endl;
    }
}

void Cgi::sendCgiBody(void) {
    if (pipe(this->_pipefd) == -1 || pipe(this->_responseFd) == -1) {
        std::cout << "webserv: pipe: " << strerror(errno) << std::endl;
        return;
    }
    write(this->_pipefd[1], this->_body.c_str(), this->_body.length());
    close(this->_pipefd[1]);
}

void Cgi::execScript(void) {
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

// Setters
void Cgi::setArgv(void) {
    this->_argv.push_back(strdup("/usr/bin/python3"));
    this->_argv.push_back(strdup("cgi-bin/upload.py"));
    this->_argv.push_back(NULL);
}

void Cgi::setEnv(void) {
    this->_env.push_back(getEnvFromHeader("CONTENT_TYPE=", "Content-Type"));
    this->_env.push_back(getEnvFromHeader("CONTENT_LENGTH=", "Content-Length"));
    this->_env.push_back(strdup("AUTH_TYPE=Basic"));
    this->_env.push_back(strdup("DOCUMENT_ROOT=./"));
    this->_env.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    this->_env.push_back(getEnvFromHeader("HTTP_COOKIE=", "Cookie"));
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

// Getters
char* Cgi::getEnvFromHeader(std::string name, std::string key) {
    size_t startPos = this->_header.find(key);
    size_t separator = this->_header.find(":", startPos);
    size_t endPos = this->_header.find("\r\n", separator);
    std::string env = name + this->_header.substr(separator + 2, endPos - separator - 2);
    return strdup(env.c_str());
}
