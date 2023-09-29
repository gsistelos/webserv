#pragma once

#include <poll.h>
#include <signal.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "Client.hpp"
#include "Error.hpp"
#include "Fd.hpp"
#include "Parser.hpp"
#include "Server.hpp"

/*
 * WebServ class is the core
 * of the program. It monitors
 * all the pollfds and call
 * servers and clients methods
 **/
class WebServ {
   public:
    WebServ(void);
    ~WebServ();

    static std::vector<pollfd> pollFds;
    static std::vector<Fd*> fds;
    static bool quit;

    static void pushPollfd(int fd);
    static void removeIndex(int index);

    void configure(const std::string& configFile);
    void start(void);
};
