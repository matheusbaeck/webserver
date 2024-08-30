#include "HttpRequest.hpp"


#if 0
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
#endif

HttpRequest::HttpRequest(const char *buffer)
{
	this->ss << buffer;
}

typedef StatusCode (HttpRequest::*parseCalls)(const std::string &token);

StatusCode HttpRequest::parseStartLine(const std::string &starLine)
{
	std::string token;
	std::stringstream ss2(starLine);
	parseCalls calls[3] = {&HttpRequest::parseMethod, &HttpRequest::parsePath, &HttpRequest::parseProtocol};

	for (size_t i = 0; i < 3; i += 1)
	{
		this->statusCode = (this->*calls[i])(ConfigFile::getToken(ss2));
		if (this->statusCode != OK) break;
	}
	return this->statusCode;
}

StatusCode HttpRequest::parseMethod(const std::string &_method)
{
	if (HttpRequest::isUpperCase(_method))
	{
		this->method = ConfigFile::isMethod(_method);
		return this->method != static_cast<Method>(-1) ? OK : NALLOWED;
	}
	return BREQUEST;
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

void	HttpRequest::parseQuery(const std::string &path)
{
	std::stringstream ss(path);
	std::string token;

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

StatusCode HttpRequest::parseProtocol(const std::string &protocol)
{
	// TODO: parse major and minor number of http version with status code 505
	return protocol == "HTTP/1.1" ? OK : BREQUEST;
}

/* --------- GETTERS ------- */

Method	HttpRequest::getMethod(void) const
{
	return this->method;
}

StatusCode HttpRequest::getStatusCode(void) const
{
	return this->statusCode;
}

StatusCode HttpRequest::parseHeaders(void)
{
	int	count = 0;
	std::string key;
	std::string value;
	Tokenizer tokenizer(this->ss);

	while (!tokenizer.end())
	{
		key = tokenizer.next(':');
		tokenizer.toLower(key);

		// key field has to be without white space
// 		if (key.find(" ") != std::string::npos)
// 			return BREQUEST;


		if (key == "host")
		{
			count += 1;

			if (this->headers.find(key) != this->headers.end())
				return BREQUEST;
			value = tokenizer.next(' ');

			// NOTE: match host with server_names
			if (!this->matchHost(value))
				return NFOUND;

			this->headers[key] = value;
		}
		if (key == "connection")
		{
			this->headers[key] = "keep-alive";
			value = tokenizer.next(' ');
			if (value == "close")
			{
				this->headers[key] = value;
			}
		}

		if (key == "content-length")
		{
			value = tokenizer.next(' ');
			if (!ConfigFile::isNumber(value))
				return BREQUEST;
			this->headers[key] = value;
		}

		if (key == "user-agent")
		{
			this->headers[key] = tokenizer.next(' ');
		}
	}

	if (this->headers.find("host") == this->headers.end())
		return BREQUEST;

	//std::cout << "size: " << this->headers.size() << std::endl;

	std::map<std::string, std::string>::iterator it = this->headers.begin();
	for (; it != this->headers.end(); it++)
	{
		std::cout << "---------------------\n";
		std::cout << "key  : " << it->first  << std::endl;
		std::cout << "value: " << it->second << std::endl;
		std::cout << "---------------------\n";

	}

	return OK;	
}

// TODO: try to put this function in configFile class
bool HttpRequest::matchHost(const std::string &host)
{
	// TODO: handle multiple servers
	std::string t = host;
	size_t found = host.find(":");
	if (found != std::string::npos)
		t = host.substr(0, found);
	std::vector<std::string> serverNames  = this->configFile.getServers()[0].getServerNames();

	return std::find(serverNames.begin(), serverNames.end(), t) != serverNames.end();
}

// TODO: what about CRLF at the EOF
void	HttpRequest::parse(void)
{


	// TODO: how we could generate a HTTP responde?

	// start line
	//std::cout << "start line status -> " << this->parseStartLine(getLine(this->ss)) << std::endl;
	//this->parseStartLine(getLine(this->ss));

	// headers
	// - must be one host field
	std::cout << this->parseHeaders() << std::endl;;
	

	
	// Body
}

/* ---------- static methods -------- */

bool HttpRequest::isUpperCase(const std::string &str)
{
	size_t count = std::count_if(str.begin(), str.end(), isupper);
	return count == str.size();
}
