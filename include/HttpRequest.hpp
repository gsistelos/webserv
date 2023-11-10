#pragma once

#include <string>

class HttpRequest {
   public:
    HttpRequest(void);
    ~HttpRequest();

    bool ready(void) const;
    bool empty(void) const;
    void clear(void);

    const std::string& getMethod(void) const;
    const std::string& getUri(void) const;
    const std::string& getQuery(void) const;
    const std::string& getBody(void) const;
    size_t getContentLength(void) const;
    std::string getHeaderValue(const std::string& key) const;

    void readRequest(int fd, int clientPos);

    class VersionNotSupported : public std::exception {
       public:
        const char* what(void) const throw();
    };

    class BadRequest : public std::exception {
       public:
        const char* what(void) const throw();
    };

   private:
    std::string _method;
    std::string _uri;
    std::string _query;

    std::string _header;
    std::string _body;

    bool _isReady;
    bool _isChunked;

    size_t _chunkSize;
    size_t _contentLength;

    std::string _chunk;

    void readHeader(int fd, int clientPos);
    void readChunkedBody(int fd, int clientPos);
    void readContentLengthBody(int fd, int clientPos);
};
