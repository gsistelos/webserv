#pragma once

#include <sys/stat.h>

#include <string>

class HttpRequest {
   public:
    HttpRequest(void);
    ~HttpRequest();

    bool ready(void);
    bool empty(void);
    void unchunkBody(void);

    const std::string& getHeader(void) const;
    const std::string& getBody(void) const;
    const std::string& getCgiFile(void) const;
    const std::string& getMethod(void) const;
    const std::string& getUri(void) const;
    const std::string& getFile(void) const;
    struct stat& getFileStat(void);
    const std::string& getQuery(void) const;

    std::string getHeaderValue(const std::string& key) const;

    void readRequest(int fd);
    void readHeader(int fd);
    void readBody(int fd);
    void setFile(const std::string& root, const std::string& file);

    void clear(void);

   private:
    std::string _header;
    std::string _body;
    bool _chunked;
    std::string _file;
    std::string _cgi_file;
    struct stat _file_stat;
    std::string _query;

    std::string _method;
    std::string _uri;

    size_t _content_length;
};
