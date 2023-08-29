#pragma once

#include <poll.h>

#include <string>
#include <vector>

#include "Socket.hpp"

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

    static std::vector<struct pollfd> pollFds;
    static std::vector<Socket*> sockets;
    static bool quit;

    static void removeIndex(int index);

    void configure(const std::string& configFile);
    void start(void);

   private:
    void createServer(std::string& fileContent);
};
