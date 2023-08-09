#pragma once

#include <netinet/in.h>

#include <vector>

class Server {
   private:
    struct sockaddr_in _address;
    socklen_t _addrlen;
    int _socketFd;

   public:
    Server(const std::string& serverAddr, int port);
    ~Server();

    int getSocketFd(void);
};
