#pragma once

#include <string>

#include "Socket.hpp"

class Server;

/*
 * Client class handles
 * requests and responses
 */
class Client : public Socket {
   public:
    Client(Server* server);
    ~Client();

    void handlePollin(int index);

   private:
    Server* _server;
    std::string _header;
    std::string _content;
    std::string _response;

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);
};
