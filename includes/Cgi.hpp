#pragma once

#include <string>
#include <vector>

class Cgi {
   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    int _requestFd[2];
    int _responseFd[2];
    const std::string& _header;
    const std::string& _body;
    std::string& _response;

   public:
    Cgi(const std::string& header, const std::string& body, std::string& response);
    ~Cgi();

    void setCgiPath(const std::string& path);
    void pushEnv(const std::string& env);
    void pushEnvFromHeader(const std::string& headerName, const std::string& envKey);

    void execScript(void);
};
