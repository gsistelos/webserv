#pragma once

#include <string>

class HttpRequest {
   public:
    HttpRequest(void);
    ~HttpRequest();

    bool ready(void) const;
    bool empty(void) const;
    void clear(void);

    const std::string& getHeader(void) const;
    const std::string& getBody(void) const;
    const std::string& getMethod(void) const;
    const std::string& getUri(void) const;
    std::string getHeaderValue(const std::string& key) const;

    void setUri(const std::string& uri);

    void readRequest(int fd);

   private:
    std::string _header;
    std::string _body;

    std::string _method;
    std::string _uri;

    bool _isChunked;

    size_t _chunkSize;
    size_t _contentLength;

    void readHeader(int fd);
    void readChunkedBody(int fd);
    void readContentLengthBody(int fd);
};
