#pragma once

#include <poll.h>
#include <netinet/in.h>

#include <sstream>
#include <vector>

enum Type {
    SERVER = 0,
    CLIENT
};

class Socket {
private:
    Type                _type;
    struct sockaddr_in _address;
    socklen_t          _addrlen;
    int                _socketFd;

public:
    Socket(std::stringstream &fileStream, std::vector<pollfd> &pollFds);
    Socket(int serverFd, std::vector<pollfd> &pollFds);
    ~Socket();

    Type getType(void);
};
