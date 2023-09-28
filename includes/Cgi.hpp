#pragma once

#include <string>
#include <vector>

class Cgi {
   public:
    Cgi(const std::string& path);
    ~Cgi();

    void setEnv(const std::string& env);
    void setBody(const std::string& body);

    std::string getResponse(void);

   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;

    std::string _body;
};
