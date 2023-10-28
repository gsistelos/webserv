#pragma once

#include <unistd.h>

#include <string>
#include <vector>

#include "Fd.hpp"

class Cgi : public Fd {
   public:
    Cgi(std::string& response);
    ~Cgi();

    void setEnv(const std::string& env);

    void exec(const std::string& path, const std::string& body);

    void handlePollin(int index);
    void handlePollout(int index);

   private:
    std::vector<char*> _env;

    int _responseFd;

    pid_t _pid;

    std::string _body;
    size_t _totalBytes;

    std::string& _response;
};
