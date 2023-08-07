#pragma once

#include "Socket.hpp"

/*
 * WebServ class is responsable for
 * connections with clients (including
 * I/O operations) and redirections
 **/
class WebServ {
   private:
    // Servers and clients pollfds
    std::vector<pollfd>  _pollFds;
    std::vector<Socket*> _sockets;

   public:
    WebServ(void);
    ~WebServ();

	void configure(const std::string &configFile);
    void start(void);
};
