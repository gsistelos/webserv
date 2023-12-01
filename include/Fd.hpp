#pragma once

/*
 * Fd class is the base
 * class for all Fds
 * (Server, Client & Cgi)
 **/
class Fd {
   public:
    Fd(void);
    virtual ~Fd();

    int getFd(void);
    virtual void routine(int index) = 0;
    virtual void handlePollin(int index) = 0;
    virtual void handlePollout(int index) = 0;

   protected:
    int _fd;
};
