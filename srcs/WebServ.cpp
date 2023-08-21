#include "WebServ.hpp"

#include <signal.h>

#include "Error.hpp"
#include "Parser.hpp"
#include "Server.hpp"

#define TIMEOUT 1 * 60 * 1000

std::vector<pollfd> WebServ::pollFds;
std::vector<Socket*> WebServ::sockets;
bool WebServ::quit = false;

void sighandler(int signo) {
    if (signo == SIGINT)
        WebServ::quit = true;
}

WebServ::WebServ(void) {
}

WebServ::~WebServ() {
    for (size_t i = 0; i < WebServ::pollFds.size(); i++) {
        WebServ::removeIndex(i);
    }
}

void WebServ::removeIndex(int index) {
    WebServ::pollFds.erase(WebServ::pollFds.begin() + index);

    delete WebServ::sockets[index];
    WebServ::sockets.erase(WebServ::sockets.begin() + index);
}

void WebServ::configure(const std::string& configFile) {
    // Set signal to quit program properly

    struct sigaction act;
    act.sa_handler = &sighandler;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &act, NULL) == -1)
        throw Error("sigaction");

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

void WebServ::start(void) {
    while (1) {
        /*
         * poll() will wait for a fd to be ready for I/O operations
         *
         * If it's the SERVER_FD, it's a incoming conenction
         * Otherwise it's incoming data from a client
         **/

        int ready = poll(WebServ::pollFds.data(), WebServ::pollFds.size(), TIMEOUT);

        if (WebServ::quit == true)
            return;

        if (ready == -1)
            throw Error("poll");

        // Iterate sockets to check if there's any incoming data

        for (size_t i = 0; i < WebServ::pollFds.size(); i++) {
            if (WebServ::pollFds[i].revents & POLLIN)
                WebServ::sockets[i]->handlePollin(i);
        }
    }
}

void WebServ::createServer(std::string& fileContent) {
    Server* server = new Server(fileContent);
    WebServ::sockets.push_back(server);

    pollfd pollFd;
    pollFd.fd = server->getFd();
    pollFd.events = POLLIN | POLLOUT;
    WebServ::pollFds.push_back(pollFd);
}
