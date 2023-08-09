#include "WebServ.hpp"

#include <signal.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "Error.hpp"
#include "Request.hpp"

#define TIMEOUT 1 * 60 * 1000
#define BUFFER_SIZE 30000

bool g_quit = false;

void sighandler(int signo) {
    if (signo == SIGINT)
        g_quit = true;
}

WebServ::WebServ(void) {
}

WebServ::~WebServ() {
    for (size_t i = 0; i < this->_sockets.size(); i++) {
        delete this->_sockets[i];
    }
}

void WebServ::configure(const std::string &configFile) {
    // Set signal to quit program properly

    g_quit = false;

    struct sigaction act;
    act.sa_handler = &sighandler;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &act, NULL) == -1)
        throw Error("Sigaction");

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
            this->_sockets.push_back(new Socket(fileStream, this->_pollFds));
        else
            throw Error("Invalid configFile token: \"" + token + "\"");
    }
}

void WebServ::start(void) {
    std::string requestBuffer;
    int responseReady = 0;
    while (1) {
        /*
         * poll() will wait for a fd to be ready for I/O operations
         *
         * If it's the SERVER_FD, it's a incoming conenction
         * Otherwise it's incoming data from a client
         **/

        int ready = poll(this->_pollFds.data(), this->_pollFds.size(), TIMEOUT);

        if (g_quit == true)
            return;

        if (ready == -1)
            throw Error("Poll");

        char readBuffer[BUFFER_SIZE + 1];

        // Iterate sockets to check if there's any incoming data

        for (size_t i = 0; i < this->_pollFds.size(); i++) {
            if (this->_pollFds[i].revents & POLLIN) {
                if (this->_sockets[i]->getType() == SERVER) {
                    // New client connecting to a server

                    std::cout << "New client connecting" << std::endl;

                    // Create new client and add it to the pollfds
                    this->_sockets.push_back(new Socket(this->_pollFds[i].fd, this->_pollFds));
                } else if (this->_sockets[i]->getType() == CLIENT) {
                    // Incoming data from client
                    std::cout << "Incoming data from client: " << i << std::endl;
                    size_t bytesRead = read(this->_pollFds[i].fd, readBuffer, BUFFER_SIZE);
                    if (bytesRead == (size_t)-1)
                        throw Error("Read");

                    if (bytesRead == 0) {
                        // Connection closed by the client

                        std::cout << "Connection closed by client: " << i << std::endl;

                        this->_pollFds.erase(this->_pollFds.begin() + i);

                        delete this->_sockets[i];
                        this->_sockets.erase(this->_sockets.begin() + i);

                        continue;
                    }

                    readBuffer[bytesRead] = '\0';
                    requestBuffer = readBuffer;

                    // If there is more to read, keep reading

                    while (bytesRead == BUFFER_SIZE) {
                        bytesRead = read(this->_pollFds[i].fd, readBuffer, BUFFER_SIZE);
                        if (bytesRead == (size_t)-1)
                            throw Error("Read");

                        readBuffer[bytesRead] = '\0';

                        requestBuffer.append(readBuffer);
                    }
                    responseReady = 1;
                }
            }
        }
        // Check sockets to see if there's any response to send
        // PS.: idk why i couldn't do this in the for above (can be the buffer loop ?)
        for (size_t i = 0; i < this->_pollFds.size(); i++) {
            if (this->_pollFds[i].revents & POLLOUT && responseReady) {
                std::cout << "Sending response to client: " << i << std::endl;
                // Process request and send response to client

                Request request(requestBuffer);

                if (write(this->_pollFds[i].fd, request.getResponse().c_str(),
                          request.getResponse().length()) == -1)
                    throw Error("Write");
                responseReady = 0;
            }
        }
    }
}
