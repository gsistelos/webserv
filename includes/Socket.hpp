#pragma once

/*
 * Socket class is the base
 * class for all sockets
 * (Server and Client)
 **/
class Socket {
   public:
    virtual ~Socket();

    int getFd(void);
    virtual void handlePollin(int index) = 0;

   protected:
    int _fd;
};
