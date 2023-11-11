#include "Fd.hpp"

#include <unistd.h>

#include <iostream>

Fd::Fd(void) : _fd(-1), _isCgi(false) {
}

Fd::~Fd() {
    if (this->_fd != -1) {
        std::cout << "Fd " << this->_fd << " closed" << std::endl;
        close(this->_fd);
    }
}

int Fd::getFd(void) {
    return this->_fd;
}

bool Fd::isCgi(void) const {
    return this->_isCgi;
}
