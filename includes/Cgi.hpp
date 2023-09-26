#pragma once

#include <string>
#include <vector>

class Cgi {
   public:
    Cgi(const std::string& path, const std::string& body);
    ~Cgi();

    void setEnv(const std::string& env);
    void setEnvFromHeader(const std::string& headerName, const std::string& envKey);

    std::string getResponse(void);

   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;

    // const std::string& _header;
    const std::string& _body;
};
