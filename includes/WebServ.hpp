#pragma once

#include <poll.h>

#include <map>

#include "Client.hpp"
#include "Server.hpp"

/*
 * WebServ class is responsable for
 * connections with clients (including
 * I/O operations) and redirections
 **/
class WebServ {
   private:
    // Servers and clients pollfds
    std::vector<pollfd> _pollFds;
    std::map<int, Server*> _servers;
    std::map<int, Client*> _clients;

   public:
    WebServ(void);
    ~WebServ();

    void configure(const std::string& configFile);
    void createServer(std::ifstream& fileStream);
    void createClient(int serverFd);
    void destroyClient(int index);
    void handlePollin(int index);
    void start(void);
};
