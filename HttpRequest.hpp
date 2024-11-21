#ifndef	HTTP_REQUEST_HPP
#define	HTTP_REQUEST_HPP

#include "ConfigFile.hpp"
#include "Selector.hpp"
#include "Tokenizer.hpp"

enum Type {NOTSET, RAW, URLENCODED, MULTIPART, JSON};
struct BodyRequest
{
	enum Type type;
	union {
		std::string *raw;
		std::map<std::string, std::string> *urlencoded;
	};

};

class HttpRequest
{
	static const char *CRLF;
	static const char *delim;
	static const size_t bufferSize = 4096;
	//static ConfigFile *configFile;

	ConfigServer *configServer;

	Tokenizer tokenizer;

    std::string serverName;
    std::string serverPort;

	// HTTP
	std::map<std::string, std::string> headers;
    std::string query;
    pid_t cgiPid;

	
	Method method;
	std::string path;
	StatusCode statusCode;
	
	std::map<std::string, std::string> mimeTypes;

	static void	toLower(char &c);

	struct BodyRequest *body;

public:
	HttpRequest(void);
	HttpRequest(const char *buffer);
	HttpRequest &operator=(const HttpRequest &other);
	HttpRequest(const HttpRequest &other);
    ~HttpRequest();

	static std::string	readFile(const char *pathname);
	static bool 		isUpperCase(const std::string &str);
	static std::string	toString(size_t num);
	static std::string 	getMimeType(std::string const &file);
	static std::string	&lower(std::string &str);

    // Setters
	// Getters
	Method 		getMethod(void) const;
    std::string getMethodStr(void)
    {
        const std::string methods[] = {"GET", "POST", "DELETE"};
        if (this->method < GET || this->method > DELETE)
            return "";
        return methods[this->method];
    }

	StatusCode 	getStatusCode(void) const;

    std::string getHeader(std::string const &key)
    {
        return this->headers[key]; 
    }

    std::string getQuery(void)
    {
        return this->query;
    }

    std::string getServerPort(void)
    {
        return this->serverPort == "" ? "80" : this->serverPort;
    }

	// method that generate HTML page of list directory.
	std::string	dirList(std::string const &dirpath);


	void	setConfig(ConfigServer &_configServer)
	{
        /*std::cout << _configServer << std::endl;*/
        this->configServer = new ConfigServer(_configServer);
		//*this->configServer = _configServer;
	}
    void    setBuffer(const char *buffer)
    {
        this->tokenizer.setBuffer(buffer);
    }

    std::string notAllowed(std::string const &str);

    std::string forbidden();
    std::string serverError();
    std::string notFound();
    std::string badRequest();


	
	// Start Line
	//StatusCode 	parseStartLine(const std::string &startLine);
	StatusCode	parseStartLine(void);
	StatusCode 	parseMethod(const std::string &_method);
	StatusCode 	parsePath(const std::string &path);
	StatusCode 	parseProtocol(const std::string &protocol);

	// Headers
	StatusCode	parseHeaders(void);
	bool		matchHost(const std::string &host);

	// body
	StatusCode	parseBody(void);

	void		parse(void);

	std::string handler(Selector& selector);
	//void	handler(void);

	std::string	GETmethod(const std::string &pathname, Route* route);
	std::string	POSTmethod(const std::string &pathname);
	std::string POSTmethodRAW(const std::string &pathname);
	std::string POSTmethodURLENCODED(const std::string &pathname);



};

#endif	// HTTP_REQUEST_HPP
