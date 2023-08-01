#include "Response.hpp"
#include <sstream>
#include <fstream>
#include <iostream> // DEBUG

Response::Response( void ) : _request(""), _response("")
{
}

Response::Response( std::string const & request ) : _request(request), _response("")
{
	std::string method;

	std::istringstream requestStream(_request);
	requestStream >> method;

	std::cout << "Method: " << method << std::endl; // DEBUG

	_request.erase(0, method.length());

	if (method == "GET")			getMethod();
	else if (method == "POST")		postMethod();
	else if (method == "DELETE")	deleteMethod();
	else							_response = "HTTP/1.1 400 Method Not Supported\n\n";
}

Response::Response( Response const & other ) : _request(other._request), _response(other._response)
{
}

Response::~Response()
{
}

Response& Response::operator=( Response const & other )
{
	_response = other._response;

	return *this;
}

std::string const & Response::getResponse( void )
{
	return _response;
}

void Response::getMethod( void )
{
	std::string page;

	std::istringstream requestStream(_request);
	requestStream >> page;

	std::cout << "Page: " << page << std::endl; // DEBUG

	if (page == "/")
		page = "./pages/index.html";
	else
		page = "./pages" + page;

	std::ifstream file(page.c_str());
	if (!file) {
		_response = "HTTP/1.1 404 Not Found\n\n";
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

		_response = responseStream.str();
	}
}

void Response::postMethod( void )
{
	_response = "HTTP/1.1 400 Method In Development";
}

void Response::deleteMethod( void )
{
	_response = "HTTP/1.1 400 Method In Development";
}
