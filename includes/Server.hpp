#pragma once

#include <string>

#include "Config.hpp"
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
    Config _config;
};
