#include "Request.hpp"

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
Request::Request(void) {
}

Request::Request(const std::string &request) {
    size_t headerEnd;

    _request = request;
    headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = request.find("\n\n");

    if (headerEnd == std::string::npos)
        this->_header = request.substr(0, headerEnd);
    else {
        this->_header = request.substr(0, headerEnd);
        this->_content = request.substr(headerEnd, request.length());
    }

    std::string method;

    std::istringstream headerStream(_header);
    headerStream >> method;

    _header.erase(0, method.length());

    if (method == "GET")
        getMethod();
    else if (method == "POST") {
        std::string eae = "cgi-bin/upload.py";
        char **argv = (char **)malloc(sizeof(char *) * 2);
        argv[0] = strdup(eae.c_str());
        argv[1] = NULL;

        char **env = (char **)malloc(sizeof(char *) * 2);
        env[0] = strdup("REQUEST_METHOD=POST");
        env[1] = NULL;

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            std::cout << "Error: pipe creation failed" << std::endl;
            exit(1);
        }

        int pid = fork();
        if (pid == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execve("cgi-bin/upload.py", argv, env) == -1) {
                std::cout << "Error: execve failed" << std::endl;
                exit(1);
            }
        } else {
            close(pipefd[0]);
            write(pipefd[1], _request.c_str(), _request.length());
            close(pipefd[1]);
            waitpid(pid, NULL, 0);
        }
    } else if (method == "DELETE")
        deleteMethod();
    else
        _response = "HTTP/1.1 400 Method Not Supported\r\n\r\n";
}

Request::~Request() {
}

const std::string &Request::getResponse(void) {
    return _response;
}

void Request::getMethod(void) {
    std::string page;

    std::istringstream headerStream(_header);
    headerStream >> page;

    if (page == "/")
        page = "./pages/home/index.html";
    else
        page = "./pages/upload/index.html";

    std::ifstream file(page.c_str());
    if (!file) {
        _response = "HTTP/1.1 404 Not Found\r\n\r\n";
        return;
    }

    // pega o conteudo da pagina html e retorna ao cliente
    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    file.close();

    std::string fileContent = contentStream.str();

    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 200 OK\r\n";
    responseStream << "Content-Type: text/html\r\n";
    responseStream << "Content-Length: " << fileContent.length() << "\r\n";
    responseStream << "\r\n";
    responseStream << fileContent;

    _response = responseStream.str();
}

void Request::postMethod(void) {
    std::string eae = "cgi-bin/upload.py";
    char **argv = (char **)malloc(sizeof(char *) * 2);
    argv[0] = strdup(eae.c_str());
    argv[1] = NULL;

    std::cout << "CONTENTTTTT: " << _content << std::endl;
    std::cout << "End content" << std::endl;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cout << "Error: pipe creation failed" << std::endl;
        exit(1);
    }

    int pid = fork();
    if (pid == 0) {
        // Ler e imprimir o conteúdo do pipe[0]
        char buffer[1024];
        int bytesRead;
        std::cout << "Start pipe 0" << std::endl
                  << std::endl;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            std::cout.write(buffer, bytesRead);
        }
        std::cout << "End pipe 0" << std::endl
                  << std::endl;
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        // if (execve("cgi-bin/upload.py", argv, NULL) == -1) {
        //     std::cout << "Error: execve failed" << std::endl;
        //     exit(1);
        // }
    } else {
        close(pipefd[0]);
        write(pipefd[1], _content.c_str(), _content.length());
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
    }
}

void Request::deleteMethod(void) {
    _response = "HTTP/1.1 400 Method In Development\r\n\r\n";
}
