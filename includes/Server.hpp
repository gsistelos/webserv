#pragma once

#include <map>

#include "Fd.hpp"
#include "Location.hpp"

typedef struct s_listen {
    unsigned int host;
    int port;
} t_listen;

/*
 * Server class store configurations
 * and accepts incoming connections
 **/
class Server : public Fd {
   public:
    Server(std::string& fileContent);
    Server(std::string& fileContent, const t_listen& listen);
    ~Server();

    const std::string* getErrorPage(int errorCode) const;
    size_t getMaxBodySize(void) const;
    const std::string& getRoot(void) const;
    const std::string* getServerName(void) const;
    const Location* getLocation(std::string uri) const;

    void handlePollin(int index);
    void handlePollout(int index);

   private:
    std::map<int, std::string> _errorPages;
    size_t _maxBodySize;
    std::vector<t_listen> _hostPort;
    std::string _root;
    std::string _serverName;
    std::map<std::string, Location> _locations;

    void configure(std::string& fileContent);
    void setErrorPage(std::string& fileContent);
    void setMaxBodySize(std::string& fileContent);
    void setHost(std::string& fileContent);
    void setListen(std::string& fileContent);
    void setRoot(std::string& fileContent);
    void setServerName(std::string& fileContent);
    void setLocation(std::string& fileContent);
};
