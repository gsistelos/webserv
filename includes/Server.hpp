#pragma once

#include <netinet/in.h>

#include <string>
#include <vector>

class Server {
   private:
    struct sockaddr_in _address;
    socklen_t _addrlen;
    std::string _ip;
    int _port;
    std::string _root;
    size_t _maxBodySize;
    int _socketFd;

    void configure(std::string& fileContent);
    void listen(std::string& fileContent);
    void root(std::string& fileContent);
    void maxBodySize(std::string& fileContent);

   public:
    Server(std::string& fileContent);
    ~Server();

    int getSocketFd(void);
    const std::string& getRoot(void);

    std::string readClientData(int clientFd);
};
