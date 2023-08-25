#include "Cgi.hpp"

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

void toUpperCase(std::string& content) {
    for (unsigned int i = 0; i < content.length(); ++i) {
        content[i] = std::toupper(content[i]);
    }
}

Cgi::Cgi(const std::string& header, const std::string& content, std::string& _response) : _header(header), _content(content), _response(_response) {
    this->setEnv();
    this->setArgv();
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

void Cgi::execScript(void) {
    if (pipe(this->_pipefd) == -1 || pipe(this->_responseFd) == -1) {
        std::cout << "webserv: pipe: " << strerror(errno) << std::endl;
        return;
    }

    int pid = fork();
    if (pid == -1) {
        std::cout << "webserv: fork: " << strerror(errno) << std::endl;
        return;
    } else if (pid == 0) {
        close(this->_pipefd[1]);
        close(this->_responseFd[0]);

        dup2(this->_pipefd[0], STDIN_FILENO);
        close(this->_pipefd[0]);

        dup2(this->_responseFd[1], STDOUT_FILENO);
        close(this->_responseFd[1]);

        execve("cgi-bin/upload.py", this->_argv.data(), this->_env.data());
        std::cout << "webserv: execve: " << strerror(errno) << std::endl;
        return;
    } else {
        // TODO: create a "setup" method that create the pipes and send the content to the cgi input
        close(this->_pipefd[0]);
        close(this->_responseFd[1]);

        if (write(this->_pipefd[1], _content.c_str(), _content.length()) == -1)
            std::cout << "webserv: write: " << strerror(errno) << std::endl;
        close(this->_pipefd[1]);

        waitpid(pid, NULL, 0);
    }
}

void Cgi::createResponse(void) {
    char buffer[30000 + 1];

    size_t bytesRead = read(this->_responseFd[0], buffer, 30000);
    close(this->_responseFd[0]);
    if (bytesRead == (size_t)-1) {
        std::cout << "webserv: read: " << strerror(errno) << std::endl;
        return;
    }
    if (bytesRead == 0) {
        this->_response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        return;
    }

    buffer[bytesRead] = '\0';

    this->_response.clear();
    this->_response.append("HTTP/1.1 200 OK\r\n");
    this->_response.append("Content-Type: text/html\r\n");
    // TODO: check if when u request a page, the content-length is the size of header + content or only content
    // and then, set the content-lenght below properly.
    std::ostringstream strBytesRead;
    strBytesRead << bytesRead;

    this->_response.append("Content-Length: " + strBytesRead.str() + "\r\n");
    this->_response.append("\r\n");
    this->_response.append(buffer);
}

void Cgi::setEnv(void) {
    this->_env.push_back(getEnvFromHeader("Content-Type"));
    this->_env.push_back(getEnvFromHeader("Content-Length"));
    this->_env.push_back(strdup("AUTH_TYPE=Basic"));
    this->_env.push_back(strdup("DOCUMENT_ROOT=./"));
    this->_env.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    // TODO: make getEnvFromHeader return only the value and set the HTTP_COOKIE manually
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

char* Cgi::getEnvFromHeader(std::string key) {
    size_t startPos = _header.find(key);
    size_t separator = _header.find(":", startPos);
    size_t endPos = _header.find("\r\n", separator);
    toUpperCase(key);
    std::string env = key + "=" + _header.substr(separator + 2, endPos - separator - 2);
    env.replace(env.find("-"), 1, "_");
    return strdup(env.c_str());
}
