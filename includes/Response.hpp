#pragma once

#include <string>

/*
 * 'Response' class is responsable for
 * interpreting the request and creating
 * a proper response to the client
 **/

class Response
{
private:
	std::string _request;
	std::string _response;

public:
	Response( void );
	Response( std::string const & request );
	Response( Response const & other );
	~Response();

	Response& operator=( Response const & other );

	std::string const & getResponse( void );

	void getMethod( void );
	void postMethod( void );
	void deleteMethod( void );
};
