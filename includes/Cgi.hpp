#pragma once

#include <iostream>
#include <vector>

class Cgi {
   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    int _pipefd[2];
    int _responseFd[2];
    std::string _response;
    std::string _header;

   public:
    Cgi();
    ~Cgi();

    // Methods
    void execScript(void);
    void buildResponse(void);
    void sendCgiBody(std::string requestContent);

    // Setters
    void setEnv(const std::string& requestHeader);
    void setArgv(void);

    // Getters
    char* getEnvFromHeader(std::string name, std::string key);
    char** getEnv(void);
    std::string getResponse(void);
    char** getArgv(void);
};
