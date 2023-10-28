#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "Client.hpp"
#include "Error.hpp"
#include "Parser.hpp"
#include "WebServ.hpp"

#define MAX_CLIENTS 128

unsigned int strToIp(std::string ip_str) {
    if (ip_str == "localhost")
        ip_str = "127.0.0.1";

    std::istringstream ss(ip_str);
    std::string octet_str;
    std::vector<unsigned int> octets;

    while (std::getline(ss, octet_str, '.')) {
        unsigned int octet;
        if (!(std::istringstream(octet_str) >> octet) || octet > 255) {
            throw Error("Invalid host");
            return 0;
        }
        octets.push_back(octet);
    }

    if (octets.size() != 4) {
        throw Error("Invalid host");
        return 0;
    }

    unsigned int ip_value = (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];

    return ip_value;
}

Server::Server(std::string& fileContent, const t_listen& hostPort) : _maxBodySize(1048576) /* 1MB */, _root("./") {
    this->configure(fileContent);

    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd == -1)
        throw Error("socket");

    int option = 1;

    if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) == -1)
        throw Error("setsockopt");

    if (fcntl(this->_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC))
        throw Error("fcntl");

    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    // Bind server socket to address and port

    address.sin_family = AF_INET;
    address.sin_port = htons(hostPort.port);
    address.sin_addr.s_addr = htonl(hostPort.host);

    if (bind(this->_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
        throw Error("bind");

    // Set server address to listen for incoming connections

    if (listen(this->_fd, MAX_CLIENTS) == -1)
        throw Error("listen");

    std::cout << "Created server: 127.0.0.1:" << hostPort.port << " on fd " << this->_fd << std::endl;
    // devo dar pushback em todos os binds de fd que faco, aqui em baixo, se nao vou monitorar apenas o ultimo listen
    WebServ::push_back(this);
}

Server::Server(std::string& fileContent) {
    std::string dummy = fileContent;
    std::string word;
    Parser::extractWord(dummy, word);
    if (word != "{")
        throw Error("Expected '{'");

    while (1) {
        Parser::extractWord(dummy, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == "}")
            break;

        if (word == "listen")
            this->setListen(dummy);
    }

    for (std::vector<t_listen>::iterator it = this->_hostPort.begin(); it != this->_hostPort.end(); it++) {
        dummy = fileContent;
        std::vector<t_listen>::iterator last = it;
        // no ultimo listen, vamos apagar o conteudo do bloco daquele server no .conf, pra conseguir ler o proximo bloco de server
        if (++last == this->_hostPort.end())
            new Server(fileContent, *it);
        else
            new Server(dummy, *it);
    }
}

Server::~Server() {
}

const std::string* Server::getErrorPage(int errorCode) const {
    if (this->_errorPages.count(errorCode) == 0)
        return NULL;
    return &this->_errorPages.at(errorCode);
}

size_t Server::getMaxBodySize(void) const {
    return this->_maxBodySize;
}

const std::string& Server::getRoot(void) const {
    return this->_root;
}

const std::string* Server::getServerName(void) const {
    if (this->_serverName.empty())
        return NULL;
    return &this->_serverName;
}

const Location* Server::getLocation(std::string uri) const {
    while (1) {
        if (this->_locations.count(uri) != 0)
            return &this->_locations.at(uri);

        if (uri[uri.length() - 1] == '/')
            uri.erase(uri.length() - 1);
        else {
            size_t pos = uri.find_last_of('/');
            if (pos == std::string::npos)
                return NULL;
            uri.erase(pos + 1);
        }
    }
}

void Server::handlePollin(int index) {
    (void)index;

    try {
        new Client(*this);
    } catch (const std::exception& e) {
        std::cerr << "webserv: " << e.what() << std::endl;
    }
}

void Server::handlePollout(int index) {
    (void)index;
}

void Server::configure(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word != "{")
        throw Error("Expecterd '{'");

    while (1) {
        Parser::extractWord(fileContent, word);
        if (word.empty())
            throw Error("Unexpected end of file");
        if (word == "}")
            break;

        if (word == "error_page")
            this->setErrorPage(fileContent);
        else if (word == "client_max_body_size")
            this->setMaxBodySize(fileContent);
        else if (word == "listen")
            // Este listen esta inutil pois ja foi utilizado no primeiro construtor
            this->setListen(fileContent);
        else if (word == "root")
            this->setRoot(fileContent);
        else if (word == "server_name")
            this->setServerName(fileContent);
        else if (word == "location")
            this->setLocation(fileContent);
        else
            throw Error("Invalid content \"" + word + "\"");
    }
}

void Server::setErrorPage(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    for (size_t i = 0; i < word.length(); i++) {
        if (std::isdigit(word[i]) == false)
            throw Error("Invalid error code");
    }

    int errorCode = std::atoi(word.c_str());
    if (errorCode < 100 || errorCode > 599)
        throw Error("Invalid error code");

    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    if (this->_errorPages.count(errorCode) != 0)
        throw Error("Duplicated error code");

    this->_errorPages[errorCode] = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::setMaxBodySize(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    size_t i = 0;
    while (i < word.length() - 1) {
        if (std::isdigit(word[i]) == false)
            throw Error("Invalid client_max_body_size");
        i++;
    }

    if (word[i] == 'K')
        this->_maxBodySize = std::atoi(word.c_str()) * 1024;
    else if (word[i] == 'M')
        this->_maxBodySize = std::atoi(word.c_str()) * 1024 * 1024;
    else if (word[i] == 'G')
        this->_maxBodySize = std::atoi(word.c_str()) * 1024 * 1024 * 1024;
    else
        throw Error("Invalid client_max_body_size");

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::setListen(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");

    unsigned int host = strToIp(word.substr(0, word.find(':')));

    word = word.substr(word.find(':') + 1, word.length() - word.find(':') - 1);
    if (word.empty())
        throw Error("You must provide a port");

    for (size_t i = 0; i < word.length(); i++) {
        if (std::isdigit(word[i]) == false)
            throw Error("Invalid server port");
    }

    int port = std::atoi(word.c_str());
    if (port < 1 || port > 65535)
        throw Error("Invalid server port");

    t_listen hostPort;
    hostPort.host = host;
    hostPort.port = port;

    this->_hostPort.push_back(hostPort);

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::setRoot(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");
    if (word == ";" || word == "{" || word == "}")
        throw Error("Expected a root");

    if (word[0] != '.' && word[0] != '/')
        word.insert(0, "./");

    this->_root = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::setServerName(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");
    if (word == ";" || word == "{" || word == "}")
        throw Error("Expected a server name");

    this->_serverName = word;

    Parser::extractWord(fileContent, word);
    if (word != ";")
        throw Error("Expected ';'");
}

void Server::setLocation(std::string& fileContent) {
    std::string word;
    Parser::extractWord(fileContent, word);
    if (word.empty())
        throw Error("Unexpected end of file");
    if (word == ";" || word == "{" || word == "}")
        throw Error("Expected a location");

    if (this->_locations.count(word))
        throw Error("Duplicated location \"" + word + "\"");

    this->_locations[word].configure(word, fileContent);
}
