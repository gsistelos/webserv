#pragma once

#include <string>

class Config {
   public:
    std::string ip;
    int port;
    std::string root;
    size_t maxBodySize;
    bool autoindex;

    Config(std::string& fileContent);
    ~Config();

   private:
    void setAutoIndex(std::string& fileContent);
    void setListen(std::string& fileContent);
    void setRoot(std::string& fileContent);
    void setMaxBodySize(std::string& fileContent);
};
