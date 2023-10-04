#pragma once

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "Cgi.hpp"
#include "Error.hpp"
#include "Fd.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

class Server;

/*
 * Client class handles
 * requests and responses
 **/
class Client : public Fd {
   public:
    Client(Server* server);
    ~Client();

    void handlePollin(int index);
    void handlePollout(void);

   private:
    Server* _server;
    HttpRequest _request;
    std::string _response;

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);
    bool isRegister(std::string uri);

    void handleDirectory(const std::string& uri);
    void getDirectoryPage(const std::string& uri);
};
