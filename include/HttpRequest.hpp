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

    void readRequest(int fd);

    class BadRequest : public std::exception {
       public:
        BadRequest(void);
        BadRequest(const std::string& message);
        ~BadRequest() throw();

        const char* what(void) const throw();

       private:
        std::string _message;
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

    void readHeader(int fd);
    void readChunkedBody(int fd);
    void readContentLengthBody(int fd);
};
