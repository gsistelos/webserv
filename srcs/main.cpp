#include <iostream>

#include "WebServ.hpp"

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " <configFile>" << std::endl;
        return 1;
    }

    WebServ webServ;

    try {
        if (argc == 2)
            webServ.configure(argv[1]);
        else
            webServ.configure("webserv.conf");
    } catch (std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
        return 1;
    }

    webServ.start();

    return 0;
}
