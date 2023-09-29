#include "Fd.hpp"

Fd::~Fd() {
    std::cout << "Fd " << this->_fd << " closed" << std::endl;
    close(this->_fd);
}

int Fd::getFd(void) {
    return this->_fd;
}
