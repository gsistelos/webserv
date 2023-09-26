#pragma once

#include <string>

class HttpRequest {
   public:
    HttpRequest(void);
    ~HttpRequest();

    bool ready(void);
    bool empty(void);

    const std::string& getHeader(void) const;
    const std::string& getBody(void) const;
    const std::string& getMethod(void) const;
    const std::string& getUri(void) const;

    std::string getHeaderValue(const std::string& key) const;

    void readRequest(int fd);
    void readHeader(int fd);
    void readBody(int fd);

    void clear(void);

   private:
    std::string _header;
    std::string _body;

    std::string _method;
    std::string _uri;

    size_t _content_length;
};
