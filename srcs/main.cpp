#include <iostream>

#include "Server.hpp"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Server server;

    try {
        server.configure("webser.conf");
        server.init();
        server.start();
    } catch (std::exception &e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
