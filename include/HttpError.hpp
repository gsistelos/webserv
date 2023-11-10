#pragma once

#include <exception>

class HttpError : public std::exception {
   public:
    HttpError(int status);
    ~HttpError() throw();

    const char* what(void) const throw();
    int status(void) const;

   private:
    const int _status;
};
