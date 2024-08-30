#ifndef	HTTP_REQUEST_HPP
#define	HTTP_REQUEST_HPP

#include "ConfigFile.hpp"
#include "Tokenizer.hpp"


class HttpRequest
{
	ConfigFile configFile;

	// HTTP
	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> queries;
	std::stringstream ss;
	// NOTE: saving PATH for later
	Method method;
	std::string path;
	StatusCode statusCode;
public:
	HttpRequest(const char *buffer);

	static bool isUpperCase(const std::string &str);

	// Getters
	Method 		getMethod(void) const;
	StatusCode 	getStatusCode(void) const;


	void		parse(void);

	
	// Start Line
	StatusCode 	parseStartLine(const std::string &startLine);
	StatusCode 	parseMethod(const std::string &_method);
	StatusCode 	parsePath(const std::string &path);
	void		parseQuery(const std::string &path);
	StatusCode 	parseProtocol(const std::string &protocol);

	// Headers
	StatusCode	parseHeaders(void);
	
	bool		matchHost(const std::string &host);

};

#endif	// HTTP_REQUEST_HPP
