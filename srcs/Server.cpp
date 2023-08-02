#include "Server.hpp"
#include "Response.hpp"
#include <iostream> // DEBUG
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define SERVER_FD 0
#define MAX_CLIENTS 128
#define TIMEOUT 1 * 60 * 1000
#define BUFFER_SIZE 1024

Server::Server( void )
{
}

Server::~Server()
{
	for (size_t i = 0; i < _fds.size(); i++) {
		close(_fds[i].fd);		
	}
}

void Server::init( std::string const & configFile )
{
	(void)configFile;

	/* TODO: handle config file */

	_port = 8080;

	std::cout << "Initializing server..." << std::endl; // DEBUG

	/* Start server as TCP/IP and set to non-block */

	_addrlen = sizeof(_address);
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) throw Server::SocketFailed();

	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

	/* Bind server address and port to socket */

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	std::memset(_address.sin_zero, '\0', sizeof _address.sin_zero);

	if (bind(serverSocket, (struct sockaddr*)&_address, _addrlen) == -1) throw Server::BindFailed();

	/* Create a pollfd to handle the serverSocket */

	pollfd serverFd;
	serverFd.fd = serverSocket;
	serverFd.events = POLLIN;

	_fds.push_back(serverFd);
}

void Server::start( void )
{
	/* Set server to listen for incoming connections */

	if (listen(_fds[SERVER_FD].fd, MAX_CLIENTS) == -1) throw Server::ListenFailed();

	while (1) {
		/*
		 * poll() will wait for a fd to be ready for I/O operations
		 *
		 * If it's the SERVER fd, it's a incoming conenction
		 * Otherwise it's incoming data from a client
		 **/

		int ready = poll(_fds.data(), _fds.size(), TIMEOUT);

		if (ready == -1)		throw Server::PollFailed();
		else if (ready == 0)	throw Server::PollTimeout();

		if (_fds[SERVER_FD].revents & POLLIN) {
			/* New client connecting to the server */

			std::cout << "New incoming connection!" << std::endl; // DEBUG

			int clientSocket = accept(_fds[SERVER_FD].fd, (struct sockaddr*)&_address, &_addrlen);
			if (clientSocket == -1) throw Server::AcceptFailed();

			pollfd clientFd;
			clientFd.fd = clientSocket;
			clientFd.events = POLLIN; // TODO: POLLOUT?

			_fds.push_back(clientFd);
		}

		char buffer[BUFFER_SIZE + 1];

		/* Iterate clients to check for events */

		for (size_t i = 1; i < _fds.size(); i++) {
			if (_fds[i].revents == 0) continue; /* No events to check */

			/* Incoming data from client */

			std::cout << "Incoming data from client index: " << i << std::endl; // DEBUG

			size_t bytesRead = read(_fds[i].fd, buffer, BUFFER_SIZE);
			if (bytesRead == (size_t)-1) throw Server::ReadFailed();

			if (bytesRead == 0) {
				/* Connection closed by the client */

				close(_fds[i].fd);
				_fds.erase(_fds.begin() + i);

				continue;
			}

			/* Read data from client and respond */

			buffer[bytesRead] = '\0';

			std::cout << "Received from client:\n" << buffer << std::endl; // DEBUG

			Response response(buffer);

			std::cout << "Response:\n" << response.getResponse() << std::endl; // DEBUG

			if (write(_fds[i].fd, response.getResponse().c_str(),
					response.getResponse().length()) == -1) throw Server::WriteFailed();
		}
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

char const * Server::PollFailed::what() const throw()
{
	return strerror(errno);
}

char const * Server::PollTimeout::what() const throw()
{
	return "Poll timeout";
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
