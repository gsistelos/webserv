#pragma once

#include <poll.h>

#include <string>
#include <vector>

/*
 * WebServ class is responsable for
 * connections with clients (including
 * I/O operations) and redirections
 **/
class WebServ {
   private:
    // Servers and clients pollfds
    std::vector<pollfd> _fds;
    std::vector<int>    _fdIdentifiers;

   public:
    WebServ(void);
    ~WebServ();

	void configure(const std::string &configFile);
    void start(void);

    void newServer(std::stringstream &fileStream);
};
