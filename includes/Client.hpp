#pragma once

#include <map>
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
    std::string _header;
    std::string _body;
    std::string _response;

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);

    std::string getHeaderValue(const std::string& header);

    static const std::string* getRedirect(const std::string& key);
};
