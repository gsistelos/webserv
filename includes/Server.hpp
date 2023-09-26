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
    const std::string* getRedirect(const std::string& uri);

    void handlePollin(int index);

   private:
    Config _config;
};
