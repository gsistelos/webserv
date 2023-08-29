#pragma once

#include <iostream>
#include <vector>

class Cgi {
   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    int _pipefd[2];
    int _responseFd[2];
    const std::string& _header;
    const std::string& _body;
    std::string& _response;

   public:
    Cgi(const std::string& header, const std::string& content, std::string& _response);
    ~Cgi();

    // Methods
    void execScript(void);
    void buildResponse(void);
    void sendCgiBody(void);

    // Setters
    void setEnv(void);
    void setArgv(void);

    // Getters
    char* getEnvFromHeader(std::string name, std::string key);
};
