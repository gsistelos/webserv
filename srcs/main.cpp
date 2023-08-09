#include <iostream>

#include "WebServ.hpp"
#include "Error.hpp"

int main(int argc, char **argv) {
    WebServ webServ;

    try {
        if (argc > 2)
            throw Error("Usage: ./webserv <configFile>");

        if (argc == 2)
            webServ.configure(argv[1]);
        else
            webServ.configure("webserv.conf");
        webServ.start();
    } catch (std::exception &e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
