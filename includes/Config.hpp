#pragma once

#include <map>
#include <string>

class Config {
   public:
    std::string ip;
    int port;
    std::string root;
    size_t maxBodySize;
    bool autoindex;
    std::map<std::string, std::string> redirects;

    Config(std::string& fileContent);
    ~Config();

   private:
    void setRedirect(std::string& fileContent, const std::string& route);
    void setLocation(std::string& fileContent);
    void setAutoIndex(std::string& fileContent);
    void setListen(std::string& fileContent);
    void setRoot(std::string& fileContent);
    void setMaxBodySize(std::string& fileContent);
};
