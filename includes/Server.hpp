#pragma once

#include <string>

#include "Config.hpp"
#include "Socket.hpp"

/*
 * Server class store configurations
 * and accepts incoming connections
 **/
class Server : public Socket {
   public:
    Server(std::string& fileContent);
    ~Server();

    const std::string& getRoot(void);
    size_t getMaxBodySize(void);
    bool getAutoindex(void);

    void handlePollin(int index);

   private:
    Config _config;
};
