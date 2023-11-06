#pragma once

#include <map>
#include <vector>

#include "Location.hpp"

typedef struct s_listen {
    unsigned int host;
    int port;
} t_listen;

class ConfigBlock {
   public:
    ConfigBlock(std::string& fileContent);
    ~ConfigBlock();

    size_t getMaxBodySize(void) const;
    const std::string& getRoot(void) const;
    const std::vector<t_listen>& getListen(void) const;
    std::vector<std::string>& getServerName(void);
    const std::string* getErrorPage(int errorCode) const;
    const Location* getLocation(std::string uri) const;

   private:
    std::pair<bool, size_t> _maxBodySize;
    std::pair<bool, std::string> _root;
    std::vector<t_listen> _listen;
    std::vector<std::string> _serverNames;
    std::map<int, std::string> _errorPages;
    std::map<std::string, Location> _locations;

    void setMaxBodySize(std::string& fileContent);
    void setRoot(std::string& fileContent);
    void setListen(std::string& fileContent);
    void setServerName(std::string& fileContent);
    void setErrorPage(std::string& fileContent);
    void setLocation(std::string& fileContent);
};
