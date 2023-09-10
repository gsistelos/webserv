#include "WebServ.hpp"

#include <signal.h>

#include <cstring>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"
#include "Server.hpp"

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
    size_t i = WebServ::pollFds.size();
    while (i--)
        WebServ::removeIndex(i);
}

void WebServ::pushPollfd(int fd) {
    pollfd pollFd;
    pollFd.fd = fd;
    pollFd.events = POLLIN | POLLOUT;
    pollFd.revents = 0;
    WebServ::pollFds.push_back(pollFd);
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
    act.sa_flags = 0;

    if (sigaction(SIGINT, &act, NULL) == -1)
        throw Error("sigaction");

    // Itarate words from config file

    std::string fileContent;
    Parser::readFile(configFile, fileContent);

    while (1) {
        std::string word;
        Parser::extractWord(fileContent, word);
        if (word.empty())
            break;

        if (word == "server")
            new Server(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

void WebServ::start(void) {
    while (1) {
        int ready = poll(WebServ::pollFds.data(), WebServ::pollFds.size(), -1);

        if (WebServ::quit == true)
            return;

        if (ready == -1) {
            std::cerr << "webserv: poll: " << strerror(errno) << std::endl;
            continue;
        }

        // Iterate sockets to check if there's any incoming data

        size_t i = WebServ::pollFds.size();
        while (i--) {
            try {
                if (WebServ::pollFds[i].revents & POLLIN)
                    WebServ::sockets[i]->handlePollin(i);
            } catch (const std::exception& e) {
                std::cerr << "webserv: " << e.what() << std::endl;
                WebServ::removeIndex(i);
            }
        }
    }
}
