
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <stdlib.h>
#include <map>

/* URI path cases:
 *	simple path: /about
 *  nested path: /products/phones
 *	path with Query parameters:	/search?q=smartphone
 *  path with resource identifier:	/users/12345
 *  path with file extension: /images/photo.jpg
 *  Path with Multiple Segments: /blog/2023/10/10/how-to-use-http
 *  Path with Special Characters: /products/featured?category=clothing&sort=price
 *	Path with a Trailing Slash: /directory/
 *	Path with Subdirectories:	/admin/settings/user-management
 *	Path with Dynamic Segments: /articles/{articleId}
 *
 * */

/*
 *	HTTP/1.0: default connection: close
 *	HTTP/1.1: default connection: keep-alive
 * */


/** ------------------- output -------------------------------- **/

// GET / HTTP/1.1\r\n
// Host: value\r\n
// Connection: value\r\n

const char *methods[]     = {"GET", "POST", "DELETE", "UNKNOWN"};
#define ARR_LEN(arr) sizeof(arr) / sizeof(arr[0])
#define PORT	2626

void	printChar(char c)
{
	std::cout << c << " -> " << static_cast<int>(c) << std::endl;
}

void	printTokens(const std::string &line)
{
	std::stringstream ss;
	std::string token;

	ss << line;

	std::cout << "[LINE]: " << line << std::endl;

	while (!ss.eof())
	{
		ss >> token;
		std::cout << "[TOKEN]: " << token << std::endl;
	}
}

void	printLines(const char *buffer)
{
	std::stringstream ss;
	std::string line;
	ss << buffer;

	while (!ss.eof())
	{
		std::getline(ss, line);
		std::cout << "[LINE]: " << line << std::endl;
	}
}

/** ------------------- output -------------------------------- **/

enum StatusCode
{
	OK 		   = 200,
	BREQUEST   = 400,
	FORBIDDEN  = 403,
	NFOUND     = 404,
	NALLOWED   = 405,
	NSUPPORTED = 505,
};

enum Method {
	GET = 0,
	POST,
	DELETE,
};

void	printBuffer(const char *buffer)
{
	std::cout << "-------------------------------------" << std::endl;
	std::cout << buffer << std::endl;
	std::cout << "-------------------------------------" << std::endl;
}

std::string toString(size_t num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

std::string getDate(void)
{
	// Date: [week day], [day of month] [month] [year] [hour:min:sec]
	time_t timestamp = time(NULL);
	struct tm dateTime = *localtime(&timestamp);
	char output[50] = {0};
	strftime(output, sizeof(output), "%a, %d %b %Y %H:%M:%S GMT", &dateTime);
	return std::string(output);
}

bool isUpperCase(const std::string &str)
{
	size_t count = std::count_if(str.begin(), str.end(), isupper);
	return count == str.size();
}

// directory where we looking for file /home/aabourri/projects/webserver/root/

#if 0
StatusCode	parsePath(const std::string &path)
{
	std::map<std::string, std::string> queries;

	if (path.find_first_of("/") == 0)
	{
		if (access(path.c_str(), F_OK) == -1)
			return NFOUND;
		
		if (path.find_first_of("?") != std::string::npos)
		{
				
		}

		return OK;
		// TODO: take the root directory from config file | default one /var/www/html/
	}
	// TODO: (status line) -> HTTP/1.1 404 Not Found | 403 Forbidden != autoindex on
	return BREQUEST;
}
#endif



std::string getLine(std::stringstream &ss)
{
	std::string line;
	std::getline(ss, line);
	return line;
}
std::string getToken(std::stringstream &ss)
{
	std::string token;
	ss >> token;
	return token;
}

std::string	readFileContent(const char *pathname)
{
	// TODO: check for errors
	std::ifstream errPage(pathname);
	std::stringstream ss;
	if (errPage.is_open())
	{
		ss << errPage.rdbuf();
	}
	return ss.str();
}

// TODO: add date header felied
// TODO: group all these function in one generic function
std::string	notAllowed(void)
{
	const std::string statusLine = "HTTP/1.1 405 Not Allowed\r\n";
	std::string headers = "Server: webserv/0.42\r\nAllow: GET, POST, DELETE\r\nContent-Type: text/html\r\n";
	std::string body = readFileContent("./err_pages/405.html");
	headers += "Content-Length: " + toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string	notFound(void)
{
	const std::string statusLine = "HTTP/1.1 404 Not Found\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = readFileContent("./err_pages/404.html");
	headers += "Content-Length: " + toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string badRequest(void)
{
	const std::string statusLine = "HTTP/1.1 400 Bad Request\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = readFileContent("./err_pages/400.html");
	headers += "Content-Length: " + toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

// StatusCode parseHeaders(const std::string headers)
// {
// 	// Host: value\r\nConnection: value\r\n
// 	if (headers.find("host:"))
// 	return OK;	
// }

std::string test_OK(const char *pathname)
{
	
	std::cout << "pathname: " << pathname << std::endl;

	const std::string statusLine = "HTTP/1.1 200 OK\r\n";
	std::string headers = "Server: webserv/0.42\r\n";
	std::string body    = readFileContent(pathname);
	headers += "Content-Length: " + toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
		
}

class HttpRequest
{
	std::map<std::string, std::string> queries;
	std::stringstream ss;
	Method method;
	// NOTE: saving PATH for later
	std::string path;
	StatusCode statusCode;
public:
	HttpRequest(const char *buffer);

	Method 		getMethod(void) const;
	StatusCode 	getStatusCode(void) const;

	void		parse(void);

	
	StatusCode 	parseStartLine(const std::string &startLine);
	StatusCode 	parseMethod(const std::string &_method);
	StatusCode 	parsePath(const std::string &path);
	void		parseQuery(const std::string &path);
	StatusCode 	parseProtocol(const std::string &protocol);
};

StatusCode HttpRequest::parseProtocol(const std::string &protocol)
{
	// TODO: parse major and minor number of http version with status code 505
	return protocol == "HTTP/1.1" ? OK : BREQUEST;
}

void	HttpRequest::parseQuery(const std::string &path)
{
	std::stringstream ss;
	std::string token;
	ss << path;

	while (!ss.eof())
	{
		std::getline(ss, token, '&');
		size_t found = token.find("=");
		if (found != std::string::npos)
		{
			this->queries[token.substr(0, found)] = token.substr(found + 1, token.size());
		}
	}
}

StatusCode HttpRequest::parsePath(const std::string &requestTarget)
{
	if (requestTarget.find_first_of("/") == 0)
	{
		size_t found = requestTarget.find_first_of("?");
		this->path   = "." + requestTarget.substr(0, found);
		if (access(this->path.c_str(), F_OK) == -1)
			return NFOUND;
		if (found != std::string::npos)
		{
			// TODO: take the root directory from config file | default one /var/www/html/
			// TODO: (status line) -> HTTP/1.1 404 Not Found | 403 Forbidden != autoindex on
			// TODO: return Bad Request if queries has errors
			this->parseQuery(requestTarget.substr(found + 1, -1));
		}
		return OK;
	}
	return BREQUEST;
}

StatusCode HttpRequest::parseMethod(const std::string &_method)
{
	const std::string methods[] = {"GET", "POST", "DELETE"};
	if (isUpperCase(_method))
	{
		for (size_t i = 0; i < ARR_LEN(methods); i += 1)
		{
			if (methods[i] == _method)
			{
				this->method = static_cast<Method>(i);
				return OK;
			}
		}
		return NALLOWED;
	}
	return BREQUEST;
}

void	HttpRequest::parse(void)
{
	// TODO: how we could generate a HTTP responde?


	// start line
	this->parseStartLine(getLine(this->ss));

	// headers
	
	while (!this->ss.eof())
	{
		std::cout << getLine(this->ss) << std::endl;
	}
	
	
	// Body
}

StatusCode HttpRequest::parseStartLine(const std::string &starLine)
{
	std::string token;
	std::stringstream ss2(starLine);

	// TODO: turn these methods to function pointer
	// 		 if find any "Bad Request" dont parse all of them
	this->statusCode = this->parseMethod(getToken(ss2));
	this->statusCode = this->parsePath(getToken(ss2));
	this->statusCode = this->parseProtocol(getToken(ss2));
	return this->statusCode;
}

HttpRequest::HttpRequest(const char *buffer)
{
	this->ss << buffer;
}

Method	HttpRequest::getMethod(void) const
{
	return this->method;
}

StatusCode HttpRequest::getStatusCode(void) const
{
	return this->statusCode;
}

void	test(std::string str)
{
	std::cout << str << std::endl;
}


int main(void)
{
	/*if (chdir("/home/aabourri/projects/webserv") == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		return 10;
	}*/

	int serverFd = socket(AF_INET, SOCK_STREAM, 0);

	if (serverFd == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		return 1;
	}

	std::cout << "server socker fd: " << serverFd << std::endl;

	int reuse = 1;

	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		return 2;
	}

	struct sockaddr_in serverAddr = {
		.sin_family = AF_INET,
		.sin_port   = htons(PORT),
		.sin_addr   = { htonl(INADDR_ANY) },
	};

	int err = bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if (err == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		if (errno != EADDRINUSE)
		{
			close(serverFd);
			return 3;
		}
	}

	err = listen(serverFd, 5);
	if (err == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		close(serverFd);
		return 4;
	}

	socklen_t clientAddrLen;
	int	clientFd;
	int sbytes, rbytes;
	(void) sbytes;
	std::cout << "Server listing on: 127.0.0.1:" << PORT << std::endl;


	clientFd = accept(serverFd, NULL, &clientAddrLen);
	if (clientFd == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		close(serverFd);
		//return 5;
	}

	std::cout << "client " << clientFd << " connected to the server" << std::endl;

	char	buffer[1024] = {0};


	rbytes = recv(clientFd, buffer, 1024, 0);

	HttpRequest httpRequest(buffer);
	httpRequest.parse();

#if 0
	printBuffer(buffer);


	if (rbytes == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		close(serverFd);
		close(clientFd);
		//return 6;
	}

// 	std::stringstream ss;
// 	ss << buffer;
// 	std::string line;// = buffer;

// 	std::getline(ss, line);

// 	std::stringstream ss2;
// 	ss2 << line;
// 	std::string token;
// 	ss2 >> token;
// 	ss2 >> token;
// 	std::string path = "." + token;

	std::string test_ = test_OK("./index.html");


	sbytes = send(clientFd, test_.c_str(), test_.size(), 0);

	if (sbytes == -1)
	{
		std::cout << "[ERROR]: " << strerror(errno) << std::endl;
		close(serverFd);
		close(clientFd);
	}
	std::cout << rbytes << ": bytes recieved from the client" << std::endl;
	// 	std::cout << sbytes << ": bytes sent to the client" << std::endl;
	///std::cout << "----------------------------------------" << std::endl;
#endif

	close(clientFd);
	close(serverFd);
	return 0;
}