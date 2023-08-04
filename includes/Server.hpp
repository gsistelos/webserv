#pragma once

#include <netinet/in.h>
#include <poll.h>

#include <exception>
#include <string>
#include <vector>

/*
 * 'Server' class is responsable for
 * connections with clients (including
 * I/O operations) and redirections
 **/

class Server {
   private:
    std::vector<pollfd> _fds;
    struct sockaddr_in _address;
    socklen_t          _addrlen;

   public:
    Server(void);
    ~Server();

    void init(const std::string &configFile);
    void start(void);

    class CustomError : public std::exception {
       private:
        const char *_message;

       public:
        CustomError(const char *message);
        const char *what() const throw();
    };
};
