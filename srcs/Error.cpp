#include "Error.hpp"

#include <errno.h>

#include <cstring>

Error::Error(void) {
    if (errno != 0)
        this->_message = strerror(errno);
}

Error::Error(const std::string &message) : _message(message) {
    if (errno != 0) {
        this->_message.append(": ");
        this->_message.append(strerror(errno));
    }
}

Error::~Error() throw() {
}

const char *Error::what(void) const throw() {
    return _message.c_str();
}
