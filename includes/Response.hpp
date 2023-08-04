#pragma once

#include <string>

/*
 * 'Response' class is responsable for
 * interpreting the request and creating
 * a proper response to the client
 **/

class Response {
   private:
    std::string _request;
    std::string _response;

   public:
    Response(void);
    Response(const std::string &request);
    Response(const Response &other);
    ~Response();

    Response &operator=(const Response &other);

    const std::string &getResponse(void);

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);
};
