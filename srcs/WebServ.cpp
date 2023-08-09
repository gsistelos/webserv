#include "WebServ.hpp"

#include <signal.h>
#include <unistd.h>

#include <cstdlib>

#include <fstream>
#include <iostream>
#include <sstream>

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
    for (size_t i = 0; i < this->_pollFds.size(); i++) {
        if (this->_clients.count(this->_pollFds[i].fd)) {
            delete this->_clients[this->_pollFds[i].fd];
        } else {
            delete this->_servers[this->_pollFds[i].fd];
        }
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
            this->createServer(fileStream);
        else
            throw Error("Invalid configFile token: \"" + token + "\"");
    }
}

void WebServ::createServer(std::stringstream &fileStream) {
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

    int port = std::atoi(portStr.c_str());

    // TODO: create a .conf class that store all server configurations, and send it to the "Server" constructor instead of server_addr and port
    Server *newServer = new Server(server_addr, port);

    // Add server fd to pollfd vector
    pollfd pollFd;
    pollFd.fd = newServer->getSocketFd();
    pollFd.events = POLLIN | POLLOUT;

    this->_pollFds.push_back(pollFd);
    this->_servers.insert(std::pair<int, Server *>(pollFd.fd, newServer));
    std::cout << "Server criado" << std::endl;
}

void WebServ::createClient(int serverFd) {
    // TODO: Store others datas inside client like request, response, etc
    Client *newClient = new Client(serverFd);

    pollfd pollFd;
    pollFd.fd = newClient->getSocketFd();
    pollFd.events = POLLIN | POLLOUT;

    this->_pollFds.push_back(pollFd);
    this->_clients.insert(std::pair<int, Client *>(pollFd.fd, newClient));
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
        if (ready == -1)
            throw Error("Poll");

        if (g_quit == true)
            return;

        char readBuffer[BUFFER_SIZE + 1];

        // Iterate sockets to check if there's any incoming data

        for (size_t i = 0; i < this->_pollFds.size(); i++) {
            if (this->_pollFds[i].revents & POLLIN && this->_servers.count(this->_pollFds[i].fd)) {
                std::cout << "Pollin no server fd: " << this->_pollFds[i].fd << std::endl;
                // Create new client and add it to the pollfds
                this->createClient(this->_pollFds[i].fd);

            } else if (this->_pollFds[i].revents & POLLIN && this->_clients.count(this->_pollFds[i].fd)) {
                // Incoming data from client
                std::cout << "Incoming data from client fd: " << this->_pollFds[i].fd << std::endl;
                size_t bytesRead = read(this->_pollFds[i].fd, readBuffer, BUFFER_SIZE);

                if (bytesRead == (size_t)-1) {
                    std::cout << "Error while reading" << std::endl;

                    this->_pollFds.erase(this->_pollFds.begin() + i);

                    delete this->_clients[this->_pollFds[i].fd];
                    this->_clients.erase(this->_pollFds[i].fd);

                    continue;
                } else if (bytesRead == 0) {
                    // Nothing to read, disconnect client
                    std::cout << "Client fd: " << this->_pollFds[i].fd << " has been disconnected" << std::endl;

                    this->_pollFds.erase(this->_pollFds.begin() + i);

                    delete this->_clients[this->_pollFds[i].fd];
                    this->_clients.erase(this->_pollFds[i].fd);

                    continue;
                }

                readBuffer[bytesRead] = '\0';
                requestBuffer = readBuffer;

                responseReady = 1;
            }
        }
        // Check sockets to see if there's any response to send
        // PS.: idk why i couldn't do this in the for above (can be the buffer loop ?)
        for (size_t i = 0; i < this->_pollFds.size(); i++) {
            if (this->_pollFds[i].revents & POLLOUT && responseReady) {
                std::cout << "Sending response to client fd: " << this->_pollFds[i].fd << std::endl;
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
