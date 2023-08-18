#include "WebServ.hpp"

#include <signal.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Error.hpp"
#include "Parser.hpp"

#define TIMEOUT 1 * 60 * 1000

bool g_quit = false;

void sighandler(int signo) {
    if (signo == SIGINT)
        g_quit = true;
}

WebServ::WebServ(void) {
}

WebServ::~WebServ() {
    for (size_t i = 0; i < this->_pollFds.size(); i++) {
        if (this->_servers.count(this->_pollFds[i].fd))
            delete this->_servers[this->_pollFds[i].fd];
        else
            delete this->_clients[this->_pollFds[i].fd];
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

    // Itarate words from config file

    std::string fileContent = Parser::readFile(configFile);

    while (1) {
        std::string word = Parser::extractWord(fileContent);
        if (word.empty())
            break;

        if (word == "server")
            this->createServer(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

void WebServ::createServer(std::string& fileContent) {
    Server* newServer = new Server(fileContent);

    pollfd pollFd;
    pollFd.fd = newServer->getSocketFd();
    pollFd.events = POLLIN | POLLOUT;

    this->_pollFds.push_back(pollFd);
    this->_servers.insert(std::pair<int, Server*>(pollFd.fd, newServer));
}

void WebServ::createClient(int serverFd) {
    Client* newClient = new Client(this->_servers[serverFd]);

    pollfd pollFd;
    pollFd.fd = newClient->getSocketFd();
    pollFd.events = POLLIN | POLLOUT;

    this->_pollFds.push_back(pollFd);
    this->_clients.insert(std::pair<int, Client*>(pollFd.fd, newClient));
}

void WebServ::destroyClient(int index) {
    delete this->_clients[this->_pollFds[index].fd];
    this->_clients.erase(this->_pollFds[index].fd);

    this->_pollFds.erase(this->_pollFds.begin() + index);
}

void WebServ::handlePollin(int index) {
    if (this->_servers.count(this->_pollFds[index].fd))
        this->createClient(this->_pollFds[index].fd);
    else {
        std::string request = this->_clients[this->_pollFds[index].fd]->getServer()->readClientData(this->_pollFds[index].fd);
        if (request.empty()) {
            this->destroyClient(index);
            return;
        }

        this->_clients[this->_pollFds[index].fd]->request(request);

        if (this->_pollFds[index].revents & POLLOUT) {
            if (write(this->_pollFds[index].fd,
                      this->_clients[this->_pollFds[index].fd]->getResponse().c_str(),
                      this->_clients[this->_pollFds[index].fd]->getResponse().length()) == -1)
                throw Error("Write");
        }
    }
}

void WebServ::start(void) {
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
            if (this->_pollFds[i].revents & POLLIN)
                handlePollin(i);
        }
    }
}
