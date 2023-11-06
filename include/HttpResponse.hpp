#pragma once

#include <string>

#include "Cgi.hpp"
#include "HttpRequest.hpp"

class HttpResponse {
   public:
    HttpResponse(void);
    ~HttpResponse();

    bool ready(void) const;
    const char* c_str(void) const;
    size_t length(void) const;
    void clear(void);

    void internalServerError(void);
    void body(int statusCode, const std::string& contentType, const std::string& body);
    void file(int statusCode, const std::string& path);
    void redirect(const std::string& redirect);
    void directoryList(const std::string& path);
    void error(int statusCode);

    void cgi(const std::string& path, const HttpRequest& request);

   private:
    std::string _response;

    std::string contentType(const std::string& filename);
    static const std::string& getStatusMessage(int statusCode);
    static const std::string& getMimeType(const std::string& extension);
};
