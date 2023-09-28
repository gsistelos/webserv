#pragma once

#include <map>
#include <string>

#include "HttpRequest.hpp"
#include "Socket.hpp"

class Server;

/*
 * Client class handles
 * requests and responses
 **/
class Client : public Socket {
   public:
    Client(Server* server);
    ~Client();

    void handlePollin(int index);

   private:
    Server* _server;
    HttpRequest _request;
    std::string _response;

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);
    bool fileSearch(std::string uri);

    void handleDirectory(const std::string& uri);
    void getDirectoryPage(const std::string& uri);
};
