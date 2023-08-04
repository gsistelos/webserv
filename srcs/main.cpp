#include <iostream>

#include "Server.hpp"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Server server;

    try {
        server.init("webserv.conf");
    } catch (std::exception &e) {
        std::cerr << "Failed to init the server: " << e.what() << std::endl;
        return 1;
    }

    try {
        server.start();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
