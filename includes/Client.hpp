#pragma once

#include <netinet/in.h>

#include <string>

class Server;

class Client {
   private:
    Server* _server;
    struct sockaddr_in _address;
    socklen_t _addrlen;
    int _socketFd;
    std::string _header;
    std::string _content;
    std::string _response;
    std::string _request;

   public:
    Client(Server* server);
    ~Client();

    Server* getServer(void);
    int getSocketFd(void);
    const std::string& getResponse(void);
    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);
    void request(const std::string& request);
};
