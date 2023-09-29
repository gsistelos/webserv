#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "Client.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "Fd.hpp"
#include "Parser.hpp"
#include "WebServ.hpp"

/*
 * Server class store configurations
 * and accepts incoming connections
 **/
class Server : public Fd {
   public:
    Server(std::string& fileContent);
    ~Server();

    const std::string& getRoot(void);
    size_t getMaxBodySize(void);
    bool getAutoindex(void);
    const std::string* getRedirect(const std::string& uri);

    void handlePollin(int index);
    void handlePollout(void);

   private:
    Config _config;
};
