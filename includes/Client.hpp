#pragma once

#include <string>

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
    std::string _request;
    size_t _headerEnd;
    std::string _response;

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);

    void getPage(const std::string& http, const std::string& uri);

    void badRequest(void);
    void notFound(void);
    void internalServerError(void);
    void notImplemented(void);

    int isRedirect(const std::string& uri);
};
