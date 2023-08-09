#pragma once

#include <netinet/in.h>

#include <vector>
#include <string>

class Server {
   private:
    struct sockaddr_in _address;
    socklen_t _addrlen;
    int _socketFd;

   public:
    Server(const std::string& serverIp, int serverPort);
    ~Server();

    int getSocketFd(void);
};
