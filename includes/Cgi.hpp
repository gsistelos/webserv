#pragma once

#include <iostream>
#include <vector>

class Cgi {
   public:
    Cgi(const std::string& header, const std::string& content, std::string& response);
    ~Cgi();

    // Getters

    char** getEnv(void);
    char** getArgv(void);
    char* getEnvFromHeader(std::string key);

    // Methods

    void createResponse(std::string& clientResponse);
    void execScript(void);

   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    int _pipefd[2];
    int _responseFd[2];
    const std::string& _header;
    const std::string& _content;
    std::string& _response;

    void setEnv(void);
    void setArgv(void);
};
