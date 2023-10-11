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

    const std::string& getErrorPage(int errorCode);
    size_t getMaxBodySize(void);
    const std::string& getRoot(void);
    const std::string& getServerName(void);

    bool getAutoIndex(const std::string& uri) const;
    const std::string* getRedirect(const std::string& uri) const;

    void handlePollin(int index);
    void handlePollout(void);

   private:
    std::map<int, std::string> _errorPages;
    size_t _maxBodySize;
    int _port;
    std::string _root;
    std::string _serverName;

    std::map<std::string, Config> _route;

    void configure(std::string& fileContent);

    void setErrorPage(std::string& fileContent);
    void setMaxBodySize(std::string& fileContent);
    void setListen(std::string& fileContent);
    void setRoot(std::string& fileContent);
    void setServerName(std::string& fileContent);
    void setLocation(std::string& fileContent);
};
