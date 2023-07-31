#include "Server.hpp"
#include <iostream> // DEBUG
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

Server::Server( void )
{
}

Server::~Server()
{
	close(_serverSocket);
}

void Server::init( std::string const & configFile )
{
	(void)configFile;

	/* TODO: handle config file */

	_port = 8080;

	std::cout << "Initializing server..." << std::endl; // DEBUG

	_addrlen = sizeof(_address);
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1) throw Server::SocketFailed();

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	std::memset(_address.sin_zero, '\0', sizeof _address.sin_zero);

	if (bind(_serverSocket, (struct sockaddr*)&_address, _addrlen) == -1) throw Server::BindFailed();
}

void Server::start( void )
{
	if (listen(_serverSocket, 3) == -1) throw Server::ListenFailed();

	while (1) {
		std::cout << "Waiting for connection..." << std::endl; // DEBUG

		int clientSocket = accept(_serverSocket, (struct sockaddr*)&_address, &_addrlen);
		if (clientSocket == -1) throw Server::AcceptFailed();

		std::cout << "Connected!" << std::endl; // DEBUG
		std::cout << "Reading from client..." << std::endl; // DEBUG

		char buffer[BUFFER_SIZE + 1];

		size_t bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
		if (bytesRead == (size_t)-1) throw Server::ReadFailed();

		buffer[bytesRead] = '\0';

		std::cout << "Received from client: \n" << buffer << std::endl; // DEBUG

		/* TODO: respond with a page */

		std::string serverMessage = "Hello from webserv!";

		if (write(clientSocket, serverMessage.c_str() , serverMessage.length()) == -1) throw Server::WriteFailed();

		close(clientSocket);

		std::cout << "Connection closed." << std::endl; // DEBUG
	}
}

char const * Server::SocketFailed::what() const throw()
{
	return strerror(errno);
}

char const * Server::BindFailed::what() const throw()
{
	return strerror(errno);
}

char const * Server::ListenFailed::what() const throw()
{
	return strerror(errno);
}

char const * Server::AcceptFailed::what() const throw()
{
	return strerror(errno);
}

char const * Server::ReadFailed::what() const throw()
{
	return strerror(errno);
}

char const * Server::WriteFailed::what() const throw()
{
	return strerror(errno);
}
