#include "Server.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "Request.hpp"

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

void Server::configure(const std::string &configFile) {
    (void)configFile;
}

void Server::init(void) {
    // Start server as TCP/IP and set to non-block

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        throw Error("socket");

    fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    this->_addrlen = sizeof(this->_address);
    std::memset(&this->_address, 0, this->_addrlen);

    // Bind server address and port to socket

    this->_address.sin_family = AF_INET;
    this->_address.sin_addr.s_addr = INADDR_ANY;
    this->_address.sin_port = htons(8080); // TODO: change port based in configFile

    if (bind(serverSocket, (struct sockaddr *)&this->_address, this->_addrlen) == -1)
        throw Error("bind");

    // Create a pollfd to handle the serverSocket

    pollfd serverFd;
    serverFd.fd = serverSocket;
    serverFd.events = POLLIN | POLLOUT;

    this->_fds.push_back(serverFd);
}

void Server::start(void) {
    // Set server to listen for incoming connections

    if (listen(this->_fds[SERVER_FD].fd, MAX_CLIENTS) == -1)
        throw Error("listen");

    while (1) {
        /*
         * poll() will wait for a fd to be ready for I/O operations
         *
         * If it's the SERVER_FD, it's a incoming conenction
         * Otherwise it's incoming data from a client
         **/

        int ready = poll(this->_fds.data(), this->_fds.size(), TIMEOUT);

        std::cout << "passed poll" << std::endl;

        if (ready == -1)
            throw Error("poll");

        if (this->_fds[SERVER_FD].revents & POLLIN) {
            // New client connecting to the server

            int clientSocket = accept(this->_fds[SERVER_FD].fd, (struct sockaddr *)&this->_address, &this->_addrlen);
            if (clientSocket == -1)
                throw Error("accept");

            pollfd clientFd;
            clientFd.fd = clientSocket;
            clientFd.events = POLLIN | POLLOUT;

            this->_fds.push_back(clientFd);
        }

        char readBuffer[BUFFER_SIZE + 1];
        std::string requestBuffer;

        // Iterate clients to check for events

        for (size_t i = 1; i < this->_fds.size(); i++) {
            if (this->_fds[i].revents == 0)
                continue;  // No events to check

            // Incoming data from client

            size_t bytesRead = read(this->_fds[i].fd, readBuffer, BUFFER_SIZE);
            if (bytesRead == (size_t)-1)
                throw Error("read");

            if (bytesRead == 0) {
                // Connection closed by the client

                close(this->_fds[i].fd);
                this->_fds.erase(this->_fds.begin() + i);

                continue;
            }

            readBuffer[bytesRead] = '\0';
            requestBuffer = readBuffer;

            // If there is more to read, keep reading

            while (bytesRead == BUFFER_SIZE) {
                bytesRead = read(this->_fds[i].fd, readBuffer, BUFFER_SIZE);
                if (bytesRead == (size_t)-1)
                    throw Error("read");

                readBuffer[bytesRead] = '\0';

                requestBuffer.append(readBuffer);
            }

            std::cout << requestBuffer << std::endl;

            Request request(requestBuffer);

            std::cout << request.getResponse() << std::endl;

            if (write(this->_fds[i].fd, request.getResponse().c_str(),
                    request.getResponse().length()) == -1)
                throw Error("write");
        }
    }
}

Server::Error::Error(const char *message) {
    std::string str = message;

    this->_message = str.c_str();
}

const char *Server::Error::what() const throw() {
    std::cerr << this->_message << ": ";

    return strerror(errno);
}
