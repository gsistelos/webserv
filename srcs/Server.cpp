#include "Server.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>  // DEBUG

#include "Response.hpp"

#define SERVER_FD 0
#define MAX_CLIENTS 128
#define TIMEOUT 1 * 60 * 1000
#define BUFFER_SIZE 30000

Server::Server(void) {
}

Server::~Server() {
    for (size_t i = 0; i < this->_fds.size(); i++) {
        close(this->_fds[i].fd);
    }
}

void Server::init(const std::string &configFile) {
    (void)configFile;

    // TODO: handle config file

    std::cout << "Initializing server..." << std::endl;  // DEBUG

    // Start server as TCP/IP and set to non-block

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        throw CustomError("socket");

    fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    this->_addrlen = sizeof(this->_address);
    std::memset(&this->_address, 0, this->_addrlen);

    // Bind server address and port to socket

    this->_address.sin_family = AF_INET;
    this->_address.sin_addr.s_addr = INADDR_ANY;
    this->_address.sin_port = htons(8080); // TODO: change port based in configFile

    if (bind(serverSocket, (struct sockaddr *)&this->_address, this->_addrlen) == -1)
        throw CustomError("bind");

    // Create a pollfd to handle the serverSocket

    pollfd serverFd;
    serverFd.fd = serverSocket;
    serverFd.events = POLLIN | POLLOUT;

    this->_fds.push_back(serverFd);
}

void Server::start(void) {
    // Set server to listen for incoming connections

    if (listen(this->_fds[SERVER_FD].fd, MAX_CLIENTS) == -1)
        throw CustomError("listen");

    while (1) {
        /*
         * poll() will wait for a fd to be ready for I/O operations
         *
         * If it's the SERVER fd, it's a incoming conenction
         * Otherwise it's incoming data from a client
         **/

        int ready = poll(this->_fds.data(), this->_fds.size(), TIMEOUT);

        if (ready == -1)
            throw CustomError("poll");

        if (this->_fds[SERVER_FD].revents & POLLIN) {
            // New client connecting to the server

            std::cout << "New incoming connection!" << std::endl;  // DEBUG

            int clientSocket = accept(this->_fds[SERVER_FD].fd, (struct sockaddr *)&this->_address, &this->_addrlen);
            if (clientSocket == -1)
                throw CustomError("accept");

            pollfd clientFd;
            clientFd.fd = clientSocket;
            clientFd.events = POLLIN | POLLOUT;

            this->_fds.push_back(clientFd);
        }

        char buffer[BUFFER_SIZE + 1];
        std::string request;
        size_t bytesRead;

        // Iterate clients to check for events

        for (size_t i = 1; i < this->_fds.size(); i++) {
            if (this->_fds[i].revents == 0)
                continue;  // No events to check

            // Incoming data from client

            std::cout << "Incoming data from client index: " << i << std::endl;

            // TODO: make it dynamic, because using loop + buffer_size untill the end is breaking the code.

            bytesRead = read(this->_fds[i].fd, buffer, BUFFER_SIZE);
            if (bytesRead == (size_t)-1)
                throw CustomError("read");

            if (bytesRead == 0) {
                // Connection closed by the client

                close(this->_fds[i].fd);
                this->_fds.erase(this->_fds.begin() + i);

                continue;
            }

            buffer[bytesRead] = '\0';

            // Read data from client and respond

            request.append(buffer);

            size_t headerEnd = request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                // Extract header content
                std::string fileContent = request.substr(headerEnd + 4);

                std::cout << "File content:\n"
                          << fileContent << std::endl;  // DEBUG

                std::ofstream outputFile("file.xml", std::ios::binary);
                outputFile.write(fileContent.c_str(), fileContent.length());
                outputFile.close();
            } else
                std::cout << "Header not found" << std::endl; // DEBUG

            std::cout << "Received from client:\n"
                      << buffer << std::endl;  // DEBUG

            Response response(buffer);

            std::cout << "Response:\n"
                      << response.getResponse() << std::endl;  // DEBUG

            if (write(this->_fds[i].fd, response.getResponse().c_str(),
                    response.getResponse().length()) == -1)
                throw CustomError("write");
        }
    }
}

Server::CustomError::CustomError(const char *message) {
    std::string str = "webserv: ";
    str.append(message);
    str.append(" syscall failed: ");
    str.append(strerror(errno));

    this->_message = str.c_str();
}

const char *Server::CustomError::what() const throw() {
    return this->_message;
}
