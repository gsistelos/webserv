#pragma once

#include "HttpResponse.hpp"
#include "Server.hpp"

/*
 * Client class handles
 * requests and responses
 **/
class Client : public Fd {
   public:
    Client(Server& server);
    ~Client();

    void handlePollin(int clientPos);
    void handlePollout(int clientPos);

   private:
    Server& _server;
    HttpRequest _request;
    HttpResponse _response;

    std::string _path;

    int parseRequest(const std::string& uri);
    int deleteFile(std::string path, const ConfigBlock& config);

    void error(int statusCode, const ConfigBlock& config);
};
