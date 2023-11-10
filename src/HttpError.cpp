#include "HttpError.hpp"

HttpError::HttpError(int status) : _status(status) {
}

HttpError::~HttpError() throw() {
}

const char* HttpError::what(void) const throw() {
    switch (this->_status) {
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 413:
            return "Payload Too Large";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 505:
            return "HTTP Version Not Supported";
        default:
            return "Unknown Error";
    }
}

int HttpError::status(void) const {
    return this->_status;
}
