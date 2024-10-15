#include "HttpRequest.hpp"
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// NOTE: connection closes if there is BAD REQUEST, otherwise depend on Connection header.

const char  *METHODS[] = {"GET", "POST", "DELETE"};
//ConfigFile   *HttpRequest::configFile;
ConfigServer *HttpRequest::configServer;
const char	*HttpRequest::delim = " \r\n";
const char	*HttpRequest::CRLF  = "\r\n";

#define THROW(str) throw std::invalid_argument(str)

template<typename T>
void	print(T const &e)
{
	std::cout << e << std::endl;
}


bool	isFile(const char *pathname)
{
	struct stat statbuf;
	stat(pathname, &statbuf);
	return S_ISREG(statbuf.st_mode);
}

bool	isDir(const char *pathname)
{
	struct stat statbuf;
	stat(pathname, &statbuf);
	return S_ISDIR(statbuf.st_mode);
}

std::vector<std::string>::const_iterator checkIndex(const std::vector<std::string> &indices)
{
	std::vector<std::string>::const_iterator it = indices.begin();
	while (it != indices.end())
	{
		if (access(it->c_str(), F_OK) == 0) return it;
		it++;
	}
	return indices.end();
}


#if 1
std::string	notAllowed(void)
{
	const std::string statusLine = "HTTP/1.1 405 Not Allowed\r\n";
	std::string headers = "Server: webserv/0.42\r\nAllow: GET, POST, DELETE\r\nContent-Type: text/html\r\n";
	std::string body = HttpRequest::readFile("./err_pages/405.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string	notFound(void)
{
	const std::string statusLine = "HTTP/1.1 404 Not Found\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/404.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string badRequest(void)
{
	const std::string statusLine = "HTTP/1.1 400 Bad Request\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/400.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string forbidden(void)
{
	const std::string statusLine = "HTTP/1.1 403 Forbidden\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/403.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
	
}
#endif

HttpRequest::HttpRequest(const HttpRequest &other)
{
	HttpRequest::operator=(other);
}

HttpRequest	&HttpRequest::operator=(const HttpRequest &other)
{
	if (this != &other)
	{
		this->headers 		= other.headers;
		this->queries		= other.queries;
		this->method  		= other.method;
		this->path    		= other.path;
		this->statusCode	= other.statusCode;
		//
		this->clientFd = other.clientFd;
		this->port = other.port;
		this->tokenizer.setBuffer(other.tokenizer.str().c_str());
	}
	return *this;
}

HttpRequest::HttpRequest(const char *buffer)
{
	std::cout << "created http request" << std::endl;
	this->tokenizer.setBuffer(buffer);
}

typedef StatusCode (HttpRequest::*parseCalls)(const std::string &token);

/*
 *	request-line = method SP request-target SP HTTP-version CRLF
 * */

StatusCode HttpRequest::parseStartLine(void)
{
	parseCalls calls[3] = {&HttpRequest::parseMethod, &HttpRequest::parsePath, &HttpRequest::parseProtocol};

	for (size_t i = 0; i < 3; i += 1)
	{
		std::string token = this->tokenizer.next(HttpRequest::delim);
		this->statusCode  = (this->*calls[i])(token);
		// TODO: make sure this condition is correct.
		if (this->statusCode != OK) return this->statusCode;
		this->tokenizer.trimSpace();
	}
	return this->tokenizer.isCRLF() ? this->statusCode : BREQUEST;
}

/*
 *	method = "GET" | "POST" | "DELETE"
 * */

StatusCode HttpRequest::parseMethod(const std::string &_method)
{
	if (!_method.empty() && HttpRequest::isUpperCase(_method))
	{
		this->method = ConfigFile::isMethod(_method);
		return this->method != static_cast<Method>(-1) ? OK : NALLOWED;
	}
	return BREQUEST;
}

/*
 * request-target = origin-form | absolute-form | authority-form | asterisk-form
 * origin-form    = "/" path [ "?" query ]
 * absolute-form  = "http" "://" host [ : port ] [ abs-path [ "?" query ]]
 * authority-form = "*"
 * */



StatusCode HttpRequest::parsePath(const std::string &requestTarget)
{
	Route *route;

	if (requestTarget.find_first_of("/") == 0)
	{
			/*
		 	*	TODO: Now let' handle one server and multiple routes.
		 	* */
		route = HttpRequest::configServer->getRoute(requestTarget);
		//route = this->configFile->getServers()[0].getRoute(requestTarget);
		std::cout << "requestTarget: " << requestTarget << std::endl;
		if (!route)
		{
			THROW("HERE");
			return NFOUND;
		}
		const std::string tmp = "." + requestTarget;
		size_t found = requestTarget.find_first_of("?");
		this->path   = tmp.substr(0, found);

		std::cout << "---------------- " << this->path << " ----------------" << std::endl;


		if (access(tmp.c_str(), F_OK) == -1) return NFOUND;
#if 1
		if (isDir(tmp.c_str()))
		{
			//std::vector<std::string> indices            = route->getIndices();
			std::vector<std::string> indices            = route->getIndex();
			std::vector<std::string>::const_iterator it = checkIndex(indices);

			if (it == indices.end())
			{
				return FORBIDDEN;
			}


			if (it != indices.end())
				this->path = tmp + *it;
		}
#endif
		if (found != std::string::npos)
		{
			// TODO: take the root directory from config file | default one /var/www/html/
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

/*
 * HTTP-version = "HTTP" "/" 1*DIGIT "." 1*DIGIT
 * */

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

StatusCode HttpRequest::parseBody(void)
{
	throw std::invalid_argument("NOT IMPLEMENTED");
	return OK;
}

StatusCode HttpRequest::parseHeaders(void)
{
	std::string key;
	std::string value;

	while (!tokenizer.end())
	{
		key = tokenizer.next(":\r\n");
		if (tokenizer.peek() == ':')
			tokenizer.get();
		HttpRequest::lower(key);
		tokenizer.trimSpace();
		value = tokenizer.next(HttpRequest::delim);

		if (key == "host")
		{
			if (value.empty() || this->headers.count(key) > 0) return BREQUEST;
			// NOTE: match host with server_names
// 			if (!this->matchHost(value))
// 				return NFOUND;

			this->headers[key] = value;
		}
		if (key == "connection")
		{
			this->headers[key] = "keep-alive";
			
			if (value == "close")
			{
				this->headers[key] = value;
			}
		}
		if (key == "content-length")
		{
			if (!ConfigFile::isNumber(value)) return BREQUEST;
// 			if (ConfigFile::toNumber(value) > this->configFile.getServers()[0].getClientBodySize())
// 			{
// 				return CTOOLARGE;
// 			}
			this->headers[key] = value;
		}
		if (key == "user-agent")
		{
			this->headers[key] = value;
		}
		
		this->tokenizer.isCRLF(); // checking "\r\n" and skip them
	}
	// *NOTE: client must send one host header.
	if (this->headers.count("host") == 0) return BREQUEST;
	return OK;	
}

// TODO: try to put this function in configFile class
bool HttpRequest::matchHost(const std::string &host)
{
	// TODO: handle multiple servers
	std::string t = host;
	size_t found = host.find(":"); // skipping port
	if (found != std::string::npos)
		t = host.substr(0, found);
	
	//std::vector<std::string> serverNames  = this->configFile->getServers()[0].getServerNames();
	std::vector<std::string> serverNames = HttpRequest::configServer->getServerNames();
	return std::find(serverNames.begin(), serverNames.end(), t) != serverNames.end();
}

// TODO: what about CRLF at the EOF
//void	HttpRequest::parse(const char *buffer)
void	HttpRequest::parse(void)
{
	// TODO: how do i generate a HTTP responde?

	/* ----------- Start Line ----------- */
	this->statusCode = this->parseStartLine();


	std::cout << "status code of start line: " << this->statusCode << std::endl;

	/* -----------   Headers  ----------- */

	/*	after ":" only space allow
	 *  no spaces allow before key of header
	 * */
	if (this->statusCode == OK)
	{
		this->statusCode = this->parseHeaders();
		std::cout << "status code of headers: " << this->statusCode << std::endl;
	}



	/* -----------   Body  ----------- */
		/* NOTE: if the body request has more client_body_max_size,
		 *  	 server should take client_body_max_size characters.
		 *		 rest of the body request should start parsing it again as new request!!
		 * */

	//this->parseBody();

	/* -----------   generate response  ----------- */

}

std::string	HttpRequest::handler(void)
{
	std::string response;

	this->parse();

	Route *route = HttpRequest::configServer->getRoute(this->path);

	switch (this->statusCode)
	{
		case BREQUEST: response = badRequest(); break;
		case NFOUND  : response = notFound();   break;
		case NALLOWED: response = notAllowed(); break;
		case OK:
			switch (this->method)
			{
				case GET:    response = this->GETmethod(this->path);            break;
				case POST:   response = notAllowed(); /*std::invalid_argument("NOT IMPLEMENTED - POST")*/;   break;
				case DELETE: std::invalid_argument("NOT IMPLEMENTED - DELETE"); break;
				default:	 std::invalid_argument("NOT IMPLEMENTED - OTHER METHOD");
			}
			break;
		case FORBIDDEN:
			if (route->getAutoIndex() && this->method == GET)
				response = this->dirList(this->path);
			else
				response = forbidden();
			break;
		default: std::invalid_argument("NOT IMPLEMENTED - STATUS CODE");
	}
	return response;
}

std::string HttpRequest::dirList(std::string const &dirpath)
{
    struct dirent *dirnt;
	struct stat	  statbuf;
	DIR	*dir;
	char buff[100];
	

	std::string startLine = "HTTP/1.1 200 OK\r\n";
	std::string headers   = "Server: webserver/0.42\r\nContent-Type: text/html\r\n";

    std::string s = "Index of " + dirpath;
    std::string body = "<html>\n<head><title>" + s + "</title></head>\n";
    body += "<body>\n<h1>" + s + "</h1><hr><pre>";

    dir = opendir(dirpath.c_str());
	//dir = opendir(std::string("." + dirpath).c_str());
    if (dir)
    {
		// TODO: do i need to pass absolute path or just relative and than i change directory?
// 		if (chdir(dirpath.c_str()) == -1)
// 			perror("chdir");
        dirnt = readdir(dir);
        while ((dirnt = readdir(dir)))
        {
			bzero(buff, sizeof(buff));
            std::string ss = dirnt->d_name;

            if (DT_DIR == dirnt->d_type)
                ss += "/";
            
            size_t len = ss.size();

            body += "\n<a href=\"" + ss + "\">" + ss + "</a>";

			if (std::string("..").compare(dirnt->d_name) != 0)
			{
				body = body + std::string(52 - len, ' ');

				if (stat(dirnt->d_name, &statbuf) == -1)
					perror("stat");
				struct tm timestamp = *localtime(&statbuf.st_mtim.tv_sec);
				strftime(buff, sizeof(buff), "%d-%b-%y %h:%m ", &timestamp);
				body += buff;

                if (DT_DIR == dirnt->d_type)
				{
					body = body + std::string(20, ' ') + '-';
				}
				else
				{
					std::string fileSize = toString(statbuf.st_size);
					body = body + std::string(21 - fileSize.size(), ' ') + fileSize;
				}
            }
        }
    	body += "\n</pre><hr></body>\n</html>";
		closedir(dir);
    }
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n\r\n";
	return startLine + headers + body;
}


/* ---------- HTTP METHODS -------- */

std::string	HttpRequest::GETmethod(const std::string &pathname)
{
	//std::string tmp = "." + pathname;

	std::cout << "pathname: " << pathname << std::endl;

	std::string statusLine = "HTTP/1.1 200 OK\r\n";
	std::string body       = HttpRequest::readFile(pathname.c_str());
	std::string headers    = "Server: webserver/0.42\r\n";
	//headers += "Date: " + 
	headers += "Content-Type: "   + HttpRequest::getMimeType(pathname) + "\r\n";
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n\r\n";

	return statusLine + headers + body;
}




/* ---------- static methods -------- */

std::string HttpRequest::readFile(const char *pathname)
{
	std::ifstream file(pathname, std::ios::in);
	std::stringstream ss;
	if (file.is_open())
	{
		ss << file.rdbuf();
		file.close();
	}
	return ss.str();
}

bool HttpRequest::isUpperCase(const std::string &str)
{
	size_t count = std::count_if(str.begin(), str.end(), isupper);
	return count == str.size();
}

std::string	HttpRequest::toString(size_t num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

void	HttpRequest::toLower(char &c)
{
	c = tolower(c);
}

std::string	&HttpRequest::lower(std::string &str)
{
	std::for_each(str.begin(), str.end(), HttpRequest::toLower);
	return str;
}

std::string	HttpRequest::getMimeType(std::string const &file)
{
	std::map<std::string, std::string> mimeTypes;
	std::string extension;

	// TODO: read them from a file.
	mimeTypes["html"] = "text/html";
	mimeTypes["txt"]  = "text/plain";
	mimeTypes["css"]  = "text/css";

	mimeTypes["jpg"]  = "image/jpeg";
	mimeTypes["jpeg"] = "image/jpeg";
	mimeTypes["png"]  = "image/png";

	mimeTypes["js"]   = "application/javascript";
	mimeTypes["json"] = "application/json";


	// NOTE: temp solution
	int	start = 0;
	if (file[0] == '.')
		start = 1;

	size_t found = file.find_first_of(".", start);

	if (found != std::string::npos)
	{
		// TODO: put substr directly as key
		extension = file.substr(found + 1, file.size());
		return mimeTypes[extension];
	}

	std::cerr << "file     : " << file      << std::endl;
	std::cerr << "extension: " << extension << std::endl; 

	return "application/octet-stream";
}

