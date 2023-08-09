#pragma once

#include <netinet/in.h>

class Client {
   private:
    struct sockaddr_in _address;
    socklen_t _addrlen;
    int _socketFd;

   public:
    Client(int serverFd);
    ~Client();

    int getSocketFd(void);
};
