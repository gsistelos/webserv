#include "Socket.hpp"

#include <unistd.h>

#include <iostream>

Socket::~Socket() {
    std::cout << "Socket " << this->_fd << " closed" << std::endl;
    close(this->_fd);
}

int Socket::getFd(void) {
    return this->_fd;
}
