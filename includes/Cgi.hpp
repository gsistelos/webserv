#pragma once

#include <iostream>
#include <vector>

class Cgi {
   public:
    Cgi();
    ~Cgi();

    // Setters

    void setEnv(std::string& _request);
    void setArgv(void);

    // Getters

    char** getEnv(void);
    char** getArgv(void);
    char* getEnvFromHeader(std::string headerName);

    // Methods

    void createResponse(std::string& clientResponse);
    void execScript(void);

   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    int _pipefd[2];
    int _responseFd[2];
    std::string _response;
    std::string _request;
};
