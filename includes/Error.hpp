#pragma once

#include <exception>
#include <string>

class Error : public std::exception {
   private:
    std::string _message;

   public:
    Error(void);
    Error(const std::string &message);
    ~Error() throw();

    const char *what() const throw();
};
