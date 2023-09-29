#pragma once

#include <unistd.h>

#include <iostream>

/*
 * Fd class is the base
 * class for all Fds
 * (Server, Clients & CGI)
 **/
class Fd {
   public:
    virtual ~Fd();

    int getFd(void);
    virtual void handlePollin(int index) = 0;
    virtual void handlePollout(void) = 0;

   protected:
    int _fd;
};
