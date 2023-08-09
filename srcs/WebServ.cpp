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
        if (this->_clients.count(this->_pollFds[i].fd))
            delete this->_clients[this->_pollFds[i].fd];
        else
            delete this->_servers[this->_pollFds[i].fd];
    }
}

void WebServ::configure(const std::string& configFile) {
    // Set signal to quit program properly

    g_quit = false;

    struct sigaction act;
    act.sa_handler = &sighandler;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &act, NULL) == -1)
        throw Error("Sigaction");

    // Itarate file tokens and set server configurations

    std::ifstream file(configFile.c_str());
    if (!file)
        throw Error("Failed to open \"" + configFile + "\"");


    std::string token;
    while (file >> token) {
        if (token == "server:")
            this->createServer(file);
        else
            throw Error("Invalid configFile token: \"" + token + "\"");
    }

    file.close();
}

void WebServ::createServer(std::ifstream& file) {
    // Extract server address and port

    std::string token;
    file >> token;

    size_t colonPos = token.find(':');
    if (colonPos == std::string::npos)
        throw Error("Invalid content next to \"server:\"");

    std::string serverIp = token.substr(0, colonPos);
    std::string port = token.substr(colonPos + 1);

    int serverPort = std::atoi(port.c_str());

    // TODO: create a .conf class that store all server configurations, and send it to the "Server" constructor instead of serverIp and serverPort

    Server* newServer = new Server(serverIp, serverPort);

    // Add server socketFd to pollfd vector

    pollfd pollFd;
    pollFd.fd = newServer->getSocketFd();
    pollFd.events = POLLIN | POLLOUT;

    this->_pollFds.push_back(pollFd);
    this->_servers.insert(std::pair<int, Server*>(pollFd.fd, newServer));
}

void WebServ::createClient(int serverFd) {
    // TODO: Store other datas inside client like request, response, etc

    Client* newClient = new Client(serverFd);

    pollfd pollFd;
    pollFd.fd = newClient->getSocketFd();
    pollFd.events = POLLIN | POLLOUT;

    this->_pollFds.push_back(pollFd);
    this->_clients.insert(std::pair<int, Client*>(pollFd.fd, newClient));
}

void WebServ::start(void) {
    char readBuffer[BUFFER_SIZE + 1];

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

        // Iterate sockets to check if there's any incoming data

        for (size_t i = 0; i < this->_pollFds.size(); i++) {
            if (this->_pollFds[i].revents & POLLIN && this->_servers.count(this->_pollFds[i].fd))
                this->createClient(this->_pollFds[i].fd);
            else if (this->_pollFds[i].revents & POLLIN && this->_clients.count(this->_pollFds[i].fd)) {
                std::cout << "Incoming data from client socketFd: " << this->_pollFds[i].fd << std::endl;

                size_t bytesRead = read(this->_pollFds[i].fd, readBuffer, BUFFER_SIZE);

                if (bytesRead == (size_t)-1) {
                    std::cout << "Error while reading" << std::endl;

                    this->_pollFds.erase(this->_pollFds.begin() + i);

                    delete this->_clients[this->_pollFds[i].fd];
                    this->_clients.erase(this->_pollFds[i].fd);

                    continue;
                } else if (bytesRead == 0) {
                    this->_pollFds.erase(this->_pollFds.begin() + i);

                    delete this->_clients[this->_pollFds[i].fd];
                    this->_clients.erase(this->_pollFds[i].fd);

                    continue;
                }

                readBuffer[bytesRead] = '\0';

                if (this->_pollFds[i].revents & POLLOUT && this->_clients.count(this->_pollFds[i].fd)) {
                    Request request(readBuffer);

                    if (write(this->_pollFds[i].fd, request.getResponse().c_str(),
                        request.getResponse().length()) == -1)
                        throw Error("Write");
                }
            }
        }
    }
}
