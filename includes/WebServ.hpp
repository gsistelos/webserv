#pragma once

#include <poll.h>

#include <string>
#include <vector>

#include "Fd.hpp"

/*
 * WebServ class is the core
 * of the program. It stores and
 * monitors all the Fds and
 * pollfds, calling Fds methods
 * when needed
 **/
class WebServ {
   public:
    WebServ(void);
    ~WebServ();

    static std::vector<pollfd> pollfds;
    static std::vector<Fd*> fds;
    static bool quit;

    static void push_back(Fd* fd);
    static void erase(int index);

    void configure(const std::string& configFile);
    void start(void);
};
