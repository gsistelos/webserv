#include "WebServ.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <sstream>

#include "Error.hpp"
#include "Request.hpp"

#define MAX_CLIENTS 128
#define TIMEOUT 1 * 60 * 1000
#define BUFFER_SIZE 30000
#define SERVER_FD 0
#define CLIENT_FD 1

WebServ::WebServ(void) {
}

WebServ::~WebServ() {
    for (size_t i = 0; i < this->_fds.size(); i++) {
        close(this->_fds[i].fd);
    }
}

void WebServ::configure(const std::string &configFile) {
    // Open configFile and read all content to a stringstream

    std::ifstream file(configFile.c_str());
    if (!file)
        throw Error("Failed to open \"" + configFile + "\"");

    std::stringstream fileStream;
    fileStream << file.rdbuf();

    file.close();

    // Itarate tokens and set server configurations

    std::string token;
    while (fileStream >> token) {
        if (token == "server:")
            newServer(fileStream);
        else
            throw Error("Invalid configFile token: \"" + token + "\"");
    }
}

void WebServ::newServer(std::stringstream &fileStream) {
    std::string token;
    fileStream >> token;

    if (fileStream.fail())
        throw Error("Invalid content next to \"server:\"");

    // Extract server address and port

    size_t colon = token.find(':');
    if (colon == std::string::npos)
        throw Error("Invalid content next to \"server:\"");

    std::string server_addr = token.substr(0, colon);
    std::string portStr = token.substr(colon + 1, token.length());

    int         port = std::atoi(portStr.c_str());

    // Start server as TCP/IP and set to non-block

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        throw Error("socket");

    fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    struct sockaddr_in address;

    std::memset(&address, 0, sizeof(address));

    // Bind server address and port to socket

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, server_addr.c_str(), &address.sin_addr) != 1)
        throw Error("Invalid server address");

    // Set server address and to listen for incoming connections

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) == -1)
        throw Error("bind");

    if (listen(serverSocket, MAX_CLIENTS) == -1)
        throw Error("Listen");

    // Add server to pollfd vector

    pollfd serverFd;
    serverFd.fd = serverSocket;
    serverFd.events = POLLIN | POLLOUT;

    this->_fds.push_back(serverFd);
    this->_fdIdentifiers.push_back(SERVER_FD);
}

void WebServ::start(void) {
    while (1) {
        /*
         * poll() will wait for a fd to be ready for I/O operations
         *
         * If it's the SERVER_FD, it's a incoming conenction
         * Otherwise it's incoming data from a client
         **/

        int ready = poll(this->_fds.data(), this->_fds.size(), TIMEOUT);

        if (ready == -1)
            throw Error("Poll");

        char readBuffer[BUFFER_SIZE + 1];
        std::string requestBuffer;

        // Iterate fds to check for events

        for (size_t i = 0; i < this->_fds.size(); i++) {
            if (this->_fds[i].revents & POLLIN && this->_fdIdentifiers[i] == SERVER_FD) {
                // New client connecting to a server

                std::cout << "New client connecting" << std::endl;

                struct sockaddr address;
                socklen_t       addrlen;

                int clientSocket = accept(this->_fds[i].fd, &address, &addrlen);
                if (clientSocket == -1)
                    throw Error("Accept");

                pollfd clientFd;
                clientFd.fd = clientSocket;
                clientFd.events = POLLIN | POLLOUT;

                this->_fds.push_back(clientFd);
                this->_fdIdentifiers.push_back(CLIENT_FD);
            }
            else if (this->_fds[i].revents & POLLIN && this->_fdIdentifiers[i] == CLIENT_FD) {
                // Incoming data from client

                std::cout << "Data incoming from client" << std::endl;

                size_t bytesRead = read(this->_fds[i].fd, readBuffer, BUFFER_SIZE);
                if (bytesRead == (size_t)-1)
                    throw Error("Read");

                if (bytesRead == 0) {
                    // Connection closed by the client

                    close(this->_fds[i].fd);
                    this->_fds.erase(this->_fds.begin() + i);
                    this->_fdIdentifiers.erase(this->_fdIdentifiers.begin() + i);

                    continue;
                }

                readBuffer[bytesRead] = '\0';
                requestBuffer = readBuffer;

                // If there is more to read, keep reading

                while (bytesRead == BUFFER_SIZE) {
                    bytesRead = read(this->_fds[i].fd, readBuffer, BUFFER_SIZE);
                    if (bytesRead == (size_t)-1)
                        throw Error("Read");

                    readBuffer[bytesRead] = '\0';

                    requestBuffer.append(readBuffer);
                }

                // Process request and send response

                Request request(requestBuffer);

                if (write(this->_fds[i].fd, request.getResponse().c_str(),
                        request.getResponse().length()) == -1)
                    throw Error("Write");
            }
        }
    }
}
