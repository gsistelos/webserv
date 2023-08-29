#pragma once

#include <string>

class Config {
   public:
    std::string ip;
    int port;
    std::string root;
    size_t maxBodySize;

    Config(std::string& fileContent);
    ~Config();

   private:
    void setListen(std::string& fileContent);
    void setRoot(std::string& fileContent);
    void setMaxBodySize(std::string& fileContent);
};
