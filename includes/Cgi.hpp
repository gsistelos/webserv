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
    std::string _request;

   public:
    Cgi();
    ~Cgi();

    void createResponse(std::string& clientResponse);
    void execScript(void);
    // Setters
    void setEnv(std::string& _request);
    void setArgv(void);

    // Getters
    char* getEnvFromHeader(std::string headerName);
    char** getEnv(void);
    char** getArgv(void);
};
