#pragma once

#include <string>

#include "Socket.hpp"

/*
 * Server class accepts
 * incoming connections
 * and read incoming data
 */
class Server : public Socket {
   public:
    Server(std::string& fileContent);
    ~Server();

    const std::string& getRoot(void);
    size_t getMaxBodySize(void);

    void handlePollin(int index);

   private:
    std::string _ip;
    int _port;
    std::string _root;
    size_t _maxBodySize;

    void configure(std::string& fileContent);
    void listen(std::string& fileContent);
    void root(std::string& fileContent);
    void maxBodySize(std::string& fileContent);
};
