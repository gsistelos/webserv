#include "WebServ.hpp"

#include <signal.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "Error.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "Cgi.hpp"

std::vector<pollfd> WebServ::pollfds;
std::vector<Fd*> WebServ::fds;
std::vector<cgiProcess> WebServ::cgiProcesses;
bool WebServ::quit = false;

void sighandler(int signo) {
    if (signo == SIGINT)
        WebServ::quit = true;
}

void addConfigToServer(const t_listen& hostPort, ConfigBlock& configBlock) {
    for (std::vector<Fd*>::iterator it = WebServ::fds.begin(); it != WebServ::fds.end(); it++) {
        Server* server = dynamic_cast<Server*>(*it);
        if (*server == hostPort) {
            server->configToServerName(configBlock);
            return;
        }
    }
    new Server(hostPort, configBlock);
}

WebServ::WebServ(void) {
}

WebServ::~WebServ() {
    size_t i = WebServ::pollfds.size();
    while (i--)
        WebServ::erase(i);
}

void WebServ::push_back(Fd* fd) {
    pollfd newPollfd;
    newPollfd.fd = fd->getFd();
    newPollfd.events = POLLIN | POLLOUT;
    newPollfd.revents = 0;

    WebServ::pollfds.push_back(newPollfd);
    WebServ::fds.push_back(fd);
}

void WebServ::erase(int index) {
    WebServ::pollfds.erase(WebServ::pollfds.begin() + index);

    delete WebServ::fds[index];
    WebServ::fds.erase(WebServ::fds.begin() + index);
}

void WebServ::configure(const std::string& configFile) {
    // Set signal to quit program properly

    struct sigaction act;

    act.sa_flags = 0;
    act.sa_handler = &sighandler;
    if (sigfillset(&act.sa_mask) != 0)
        throw Error("sigfillset");

    if (sigaction(SIGINT, &act, NULL) != 0)
        throw Error("sigaction");

    // Itarate words from config file

    std::string fileContent;
    Parser::readFile(configFile, fileContent);

    try {
        std::vector<ConfigBlock*> configBlocks;
        while (1) {
            std::string word;
            Parser::extractWord(fileContent, word);
            if (word.empty())
                break;

            if (word == "server")
                configBlocks.push_back(new ConfigBlock(fileContent));
            else
                throw Error("invalid content \"" + word + "\"");
        }
        for (size_t i = 0; i < configBlocks.size(); i++) {
            const std::vector<t_listen>& listen = configBlocks[i]->getListen();
            for (size_t j = 0; j < listen.size(); j++) {
                addConfigToServer(listen[j], *configBlocks[i]);
            }
        }
    } catch (const std::exception& e) {
        throw Error(configFile + ": " + e.what());
    }
}

void WebServ::checkRunningProcesses(int index) {
    time_t current_time = time(NULL);

    for (std::vector<cgiProcess>::iterator it = WebServ::cgiProcesses.begin(); it != WebServ::cgiProcesses.end();) {
        time_t elapsed_time = current_time - it->start_time;

        if (elapsed_time > 2) {
            kill(it->pid, SIGKILL);
            WebServ::erase(index);
            it = WebServ::cgiProcesses.erase(it);
        } else {
            ++it;
        }
    }
}

void WebServ::start(void) {
    while (1) {
        int ready = poll(WebServ::pollfds.data(), WebServ::pollfds.size(), -1);

        if (WebServ::quit == true)
            return;

        if (ready == -1) {
            std::cerr << "webserv: poll: " << strerror(errno) << std::endl;
            continue;
        }

        // Iterate sockets to check if there's any incoming data

        size_t i = WebServ::pollfds.size();
        while (i--) {
            try {
                if (WebServ::fds[i]->isCgi())
                    this->checkRunningProcesses(i);
                if (WebServ::pollfds[i].revents & POLLIN)
                    WebServ::fds[i]->handlePollin(i);
                else if (WebServ::pollfds[i].revents & POLLOUT)
                    WebServ::fds[i]->handlePollout(i);
            } catch (const std::exception& e) {
                std::cerr << "webserv: " << e.what() << std::endl;
            }
        }
    }
}
