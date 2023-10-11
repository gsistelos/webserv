#pragma once

#include <map>
#include <string>
#include <vector>

#define GET 0x01
#define POST 0x02
#define DELETE 0x04

class Config {
   public:
    int availableMethods;
    std::string redirect;
    std::string alias;
    bool autoIndex;
    std::string index;
    std::vector<std::string> cgiExtensions;
    bool canUpload;
    std::string uploadPath;

    Config(void);
    ~Config();

    void configure(std::string& fileContent);

   private:
    void setAvailableMethods(std::string& fileContent);
    void setRedirect(std::string& fileContent);
    void setAlias(std::string& fileContent);
    void setAutoIndex(std::string& fileContent);
    void setIndex(std::string& fileContent);
    void setCgiExtensions(std::string& fileContent);
    void setCanUpload(std::string& fileContent);
    void setUploadPath(std::string& fileContent);
};
