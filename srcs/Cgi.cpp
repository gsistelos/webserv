#include "Cgi.hpp"

#include <cstdlib>
#include <cstring>

Cgi::Cgi(void) {
}

Cgi::~Cgi(void) {
    for (std::vector<char*>::iterator it = this->_argv.begin(); it != this->_argv.end(); ++it)
        free(*it);
    for (std::vector<char*>::iterator it = this->_env.begin(); it != this->_env.end(); ++it)
        free(*it);
}

void toUpperCase(std::string& content) {
    for (unsigned int i = 0; i < content.length(); ++i) {
        content[i] = std::toupper(content[i]);
    }
}

// Setters
void Cgi::setArgv(void) {
    this->_argv.push_back(strdup("cgi-bin/upload.py"));
    this->_argv.push_back(NULL);
}

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
