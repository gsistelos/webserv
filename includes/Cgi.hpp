#pragma once

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "Error.hpp"
#include "Fd.hpp"
#include "WebServ.hpp"

class Cgi : public Fd {
   public:
    Cgi(const std::string& path, const std::string& body, std::string& response);
    ~Cgi();

    void setEnv(const std::string& env);

    void execScript(void);
    void startPipes(void);
    void handlePollin(int index);
    void handlePollout(void);
    void sendBody(void);

   private:
    std::vector<char*> _argv;
    std::vector<char*> _env;
    pid_t _pid;
    int _requestFd[2];
    int _responseFd[2];

    const std::string& _body;
    std::string& _response;
};
