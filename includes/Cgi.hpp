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
    std::string _content;

   public:
    Cgi();
    ~Cgi();

    // Methods
    void execScript(const std::string& requestContent);
    void createResponse(std::string& clientResponse);

    // Setters
    void setEnv(const std::string& requestHeader);
    void setArgv(void);

    // Getters
    char* getEnvFromHeader(std::string headerName);
    char** getEnv(void);
    char** getArgv(void);
};
