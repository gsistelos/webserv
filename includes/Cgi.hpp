#pragma once

#include <iostream>
#include <vector>

class Cgi {
   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    int _pipefd[2];
    int _responseFd[2];
    const std::string& _request;
    std::string& _response;

   public:
    Cgi(const std::string& request, std::string& response);
    ~Cgi();

    void pushArgv(const std::string& argv);
    void pushEnv(const std::string& env);

    void execScript(void);
    void buildResponse(void);
};
