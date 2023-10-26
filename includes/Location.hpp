#pragma once

#include <string>

class Location {
   public:
    Location(void);
    ~Location();

    const std::string& getUri(void) const;
    bool isMethodAllowed(const std::string& method) const;
    const std::string* getRedirect(void) const;
    const std::string* getAlias(void) const;
    bool getAutoindex(void) const;
    const std::string& getIndex(void) const;
    bool isCgiExtension(const std::string& filename) const;
    bool getCanUpload(void) const;
    const std::string& getUploadPath(void) const;

    void configure(const std::string& uri, std::string& fileContent);

   private:
    std::string uri;
    std::string allowedMethods;
    std::string redirect;
    std::string alias;
    bool autoindex;
    std::string index;
    std::string cgiExtensions;
    bool canUpload;
    std::string uploadPath;

    void setAllowMethods(std::string& fileContent);
    void setRedirect(std::string& fileContent);
    void setAlias(std::string& fileContent);
    void setAutoindex(std::string& fileContent);
    void setIndex(std::string& fileContent);
    void setCgiExtensions(std::string& fileContent);
    void setCanUpload(std::string& fileContent);
    void setUploadPath(std::string& fileContent);
};
