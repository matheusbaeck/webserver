#ifndef	HTTP_REQUEST_HPP
#define	HTTP_REQUEST_HPP

#include "ConfigFile.hpp"
#include "Tokenizer.hpp"


class HttpRequest
{
	static const char *CRLF;
	static const char *delim;
	static const size_t bufferSize = 4096;
	static ConfigFile *configFile;
	//ConfigFile configFile;
	
	Tokenizer tokenizer;
	
	int	clientFd;
	int	port;

	// HTTP
	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> queries;

	
	Method method;
	std::string path;
	StatusCode statusCode;
	
	std::map<std::string, std::string> mimeTypes;

	static void	toLower(char &c);

public:
	HttpRequest(void) {}
	HttpRequest(int clientFd, int port);
	HttpRequest(const char *buffer);
	HttpRequest &operator=(const HttpRequest &other);
	HttpRequest(const HttpRequest &other);

	static std::string	readFile(const char *pathname);
	static bool 				isUpperCase(const std::string &str);
	static std::string	toString(size_t num);
	static std::string 	getMimeType(std::string const &file);
	static std::string	&lower(std::string &str);

	// Getters
	Method 		getMethod(void) const;
	StatusCode 	getStatusCode(void) const;


	// method that generate HTML page of list directory.
	std::string	dirList(std::string const &dirpath);


	void	setConfigFile(ConfigFile &_configFile)
	{
		HttpRequest::configFile = &_configFile;
	}


	
	// Start Line
	//StatusCode 	parseStartLine(const std::string &startLine);
	StatusCode	parseStartLine(void);
	StatusCode 	parseMethod(const std::string &_method);
	StatusCode 	parsePath(const std::string &path);
	void		parseQuery(const std::string &path);
	StatusCode 	parseProtocol(const std::string &protocol);

	// Headers
	StatusCode	parseHeaders(void);
	bool		matchHost(const std::string &host);

	// body
	StatusCode	parseBody(void);

	void		parse(void);

	std::string handler(void);
	//void	handler(void);

	std::string	GETmethod(const std::string &pathname);
};

#endif	// HTTP_REQUEST_HPP
