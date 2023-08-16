#pragma once

#include <string>

/*
 * Request class is responsable for
 * interpreting the request and creating
 * a proper response to the client
 **/
class Request {
   private:
    std::string _header;
    std::string _content;
    std::string _response;
    std::string _request;

   public:
    Request(void);
    Request(const std::string& request);
    ~Request();

    const std::string& getResponse(void);

    void getMethod(void);
    void postMethod(void);
    void deleteMethod(void);
};
