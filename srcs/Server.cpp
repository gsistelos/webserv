#include "Server.hpp"
#include <iostream> // DEBUG
#include <sstream>
#include <fstream>
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

		sendResponse(clientSocket, buffer);

		close(clientSocket);

		std::cout << "Connection closed." << std::endl; // DEBUG
	}
}

void Server::sendResponse( int clientSocket, char const * requestBuffer )
{
	std::string method, page, http;

	std::istringstream requestStream(requestBuffer);
	requestStream >> method >> page >> http;

	/* TODO: handle method */

	if (page == "/")
		page = "./pages/index.html";
	else
		page = "./pages" + page;

	// std::cout << method << " " << page << " " << http << std::endl; // DEBUG

	std::string response;

	std::ifstream file(page.c_str());
	if (!file) {
		response = "HTTP/1.1 404 Not Found\n\n";
	} else {
		std::ostringstream contentStream;
		contentStream << file.rdbuf();
		file.close();

		std::string fileContent = contentStream.str();

		std::ostringstream responseStream;
		responseStream << "HTTP/1.1 200 OK\n";
		responseStream << "Content-Type: text/html\n";
		responseStream << "Content-Length: " << fileContent.length() << "\n";
		responseStream << "\n";
		responseStream << fileContent;

		response = responseStream.str();
	}

	std::cout << "Response: " << response << std::endl; // DEBUG

	if (write(clientSocket, response.c_str(), response.length()) == -1) throw Server::WriteFailed();
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
