#pragma once

#include <map>
#include <string>

class HttpResponse {
   public:
    HttpResponse(void);
    ~HttpResponse();

    static const char internalServerError[];

    void setStatusCode(int statusCode);
    void setHeader(const std::string& key, const std::string& value);
    void setHeader(const std::string& key, size_t value);
    void setBody(const std::string& body);

    std::string toString(void) const;

    static std::string pageResponse(int status, const std::string& uri);
    static std::string redirectResponse(const std::string& uri);

    static std::string contentType(const std::string& uri);

   private:
    int _statusCode;
    std::map<std::string, std::string> _headers;
    std::string _body;

    static const std::string& getStatusMessage(int statusCode);
    static const std::string& getMimeType(const std::string& extension);
};
