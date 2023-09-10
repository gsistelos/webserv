#pragma once

#include <exception>
#include <string>

class Error : public std::exception {
   public:
    Error(void);
    Error(const std::string& message);
    ~Error() throw();

    const char* what() const throw();

   private:
    std::string _message;
};
