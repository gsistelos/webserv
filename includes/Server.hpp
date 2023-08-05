#pragma once

#include <netinet/in.h>
#include <poll.h>

#include <exception>
#include <string>
#include <vector>

// Vector position of server's pollfd
#define SERVER_FD 0

/*
 * Server class is responsable for
 * connections with clients (including
 * I/O operations) and redirections
 **/
class Server {
   private:
    // Clients pollfds
    std::vector<pollfd> _fds;

    struct sockaddr_in _address;
    socklen_t          _addrlen;

   public:
    Server(void);
    ~Server();

	void configure(const std::string &configFile);
    void init(void);
    void start(void);

    class Error : public std::exception {
       private:
        const char *_message;

       public:
        Error(const char *message);
        const char *what() const throw();
    };
};
