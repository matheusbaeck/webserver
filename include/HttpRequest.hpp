#ifndef	HTTP_REQUEST_HPP
#define	HTTP_REQUEST_HPP

#include "ConfigFile.hpp"
#include "Selector.hpp"
#include "Tokenizer.hpp"

enum Type {NOTSET, RAW, MULTIPART, URLENCODED};
class BodyRequest
{
public:
	enum Type   type;
    size_t      size;
    std::string raw;
    std::map<std::string, std::string> urlencoded;
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

    bool bufferFlag;

    std::string response;
	
	Method method;
	std::string path;
	StatusCode statusCode;
	
	std::map<std::string, std::string> mimeTypes;

	static void	toLower(char &c);

	struct BodyRequest body;
    std::string _targetRequest;

    size_t pos;


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
    int                 _bodyPipe[2];

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

    size_t getPos(void) 
    {
        return this->pos;
    }
    void    incrementPos(size_t incrPos)
    {
        this->pos += incrPos;
    }

	StatusCode 	getStatusCode(void) const;

    std::string getHeader(std::string const &key)
    {
        return this->headers[key]; 
    }
    std::string& getResponse()
    {
        return this->response;
    }
    struct BodyRequest& getBody()
    {
        return this->body;
    }

    std::string getQuery(void)
    {
        return this->query;
    }

    bool getBufferFlag(void)
    {
        return this->bufferFlag;
    }
    void setBufferFlag(bool flag)
    {
        this->bufferFlag = flag;
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

    static std::string notAllowed(std::string const &str, std::string const& errPage);
    static std::string gatewayTimeout(std::string const& errPage);
    static std::string forbidden(std::string const& errPage);
    static std::string serverError(std::string const& errPage);
    static std::string notFound(std::string const& errPage);
    static std::string badRequest(std::string const& errPage);
    static std::string requestTimeout(std::string const& errPage);
    static std::string payloadTooLarge(std::string const& errPage);
    static std::string createdFile(std::string filename, std::string const& errPage);
    
    std::string getStatusLine(StatusCode statusCode);

	
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
    std::string getBoundaryMultipart(std::stringstream& ss);
    std::string getFilenameMultipart(std::stringstream& ss);
    std::string getContentTypeMultipart(std::stringstream& ss);
    std::string createAbsoluteFilePath(const std::string& pathname, const std::string& filename);


	void		parse(void);

	std::string handler(Selector& selector, int clientFd);

	std::string	GETmethod(std::string &pathname, Route *route);
	std::string	DELETEmethod(const std::string &pathname);
	std::string	POSTmethod(const std::string &pathname);
	std::string POSTmethodRAW(const std::string &pathname);
	std::string POSTmethodURLENCODED(const std::string &pathname);
    std::string POSTmethodMULTIPART(const std::string& pathname);



};

#endif	// HTTP_REQUEST_HPP
