#include "HttpRequest.hpp"
#include "CgiHandler.hpp"

#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>

// NOTE: connection closes if there is BAD REQUEST, otherwise depend on Connection header.

const char  *METHODS[] = {"GET", "POST", "DELETE"};
//ConfigFile   *HttpRequest::configFile;
//ConfigServer *HttpRequest::configServer;
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




#if 1
std::string	HttpRequest::notAllowed(std::string const &str)
{
	const std::string statusLine = "HTTP/1.1 405 Not Allowed\r\n";

	std::string headers = "Server: webserv/0.42\r\n";
	headers += str + "\r\n";
	headers += "Content-Type: text/html\r\n";

	std::string body = HttpRequest::readFile("./err_pages/405.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string	HttpRequest::serverError(void)
{
	const std::string statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/500.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string	HttpRequest::notFound(void)
{
	const std::string statusLine = "HTTP/1.1 404 Not Found\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/404.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}


std::string	HttpRequest::gatewayTimeout(void)
{
        std::string statusLine = "HTTP/1.1 504 Gateway Timeout\r\n";
	    std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	    std::string body    = HttpRequest::readFile("./err_pages/504.html");
	    headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	    headers += "Connection: close\r\n\r\n";
        return statusLine + headers + body + "\r\n";
}

std::string HttpRequest::badRequest(void)
{
	const std::string statusLine = "HTTP/1.1 400 Bad Request\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/400.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}


std::string HttpRequest::requestTimeout(void)
{
	const std::string statusLine = "HTTP/1.1 408 Forbidden\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/408.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}

std::string HttpRequest::forbidden(void)
{
	const std::string statusLine = "HTTP/1.1 403 Forbidden\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/403.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}


std::string HttpRequest::payloadTooLarge(void)
{
	const std::string statusLine = "HTTP/1.1 413 Payload Too Large\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
	std::string body    = HttpRequest::readFile("./err_pages/413.html");
	headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n";
	headers += "Connection: close\r\n\r\n";
	return statusLine + headers + body + "\r\n";
}
	

#endif

HttpRequest::HttpRequest(void) 
{
    this->pos  = 0;
    this->bufferFlag = false;
    this->headers["connection"] = "keep-alive";
    pipe(_bodyPipe);
}

HttpRequest::HttpRequest(const HttpRequest &other)
{
	HttpRequest::operator=(other);
}

HttpRequest	&HttpRequest::operator=(const HttpRequest &other)
{
	if (this != &other)
	{
		this->headers 		= other.headers;
		this->query		    = other.query;
		this->method  		= other.method;
		this->path    		= other.path;
		this->statusCode	= other.statusCode;
        this->_targetRequest = other._targetRequest;
		this->tokenizer.setBuffer(other.tokenizer.str().c_str());
	}
	return *this;
}

HttpRequest::~HttpRequest()
{
    /*if (this->body->size)*/
    /*{*/
    /*    if (this->body->raw)*/
    /*        delete this->body->raw;*/
    /*    else if (this->body->urlencoded)*/
    /*        delete this->body->urlencoded;*/
    /*    delete this->body;*/
    /*}*/
    //delete this->body->raw;
    //delete this->body;
    delete this->configServer;
}

HttpRequest::HttpRequest(const char *buffer)
{
	this->tokenizer.setBuffer(buffer);
}

typedef StatusCode (HttpRequest::*parseCalls)(const std::string &token);

/*
 *	request-line = method SP request-target SP HTTP-version CRLF is not a valid subnet mask:

The binary representation 00101101 does not have a contiguous sequence of 1s followed by 0s.
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

std::string findCGIScript(const std::string& cgi_path, const std::vector<std::string>& cgi_extensions) 
{
    DIR* dir = opendir(cgi_path.c_str());
    if (dir == NULL) {
        // Error opening the directory
        return "";
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        std::string filename = entry->d_name;
        for (size_t i = 0; i < cgi_extensions.size(); i++) 
        {
            if (filename.length() >= cgi_extensions[i].length() 
                    && filename.substr(filename.length() - cgi_extensions[i].length()) == cgi_extensions[i]) 
            {
                std::cout << "filename foundScript: " << filename << std::endl;
                closedir(dir);
                return filename;
            }
        }
    }

    closedir(dir);
    return ""; // No matching CGI script found
}


std::vector<std::string>::const_iterator checkIndex(const std::vector<std::string> &indices)
{
	std::vector<std::string>::const_iterator it = indices.begin();
	while (it != indices.end())
	{
        std::cout << "trying to access: " << it->c_str() << std::endl;
		if (access(it->c_str(), F_OK) == 0) return it;
		it++;
	}
	return indices.end();
}

std::vector<std::string>::const_iterator findIndex(std::string const& root, const std::vector<std::string> &indices)
{
    
	std::vector<std::string>::const_iterator it = indices.begin();
	while (it != indices.end())
	{
        std::string index = *it;
        std::string check = (root + "/" + index).c_str();
        std::cout << "findIndex check: "<<check << std::endl;
		if (access(check.c_str(), F_OK) == 0) 
            return it;
		it++;
	}
	return indices.end();
}

StatusCode HttpRequest::parsePath(const std::string &requestTarget)
{
	Route                           *route;
    size_t                          found;
    std::string                     target;
    std::string                     fullPath;
    std::vector<Method>::iterator   it;

	if (requestTarget.find_first_of("/") == 0)
	{
        route = HttpRequest::configServer->getRoute(requestTarget);
        if (!route)
            return NFOUND;
        this->_targetRequest = requestTarget;;
        if (route->getMethods().size())
        {
            it = std::find(route->getMethods().begin(), route->getMethods().end(), this->method);
            if (it == route->getMethods().end()) 
            {
                route->setAutoIndex(false);
                return FORBIDDEN;
            }
        }

        if (route->isCgi())
        {
            found = requestTarget.find_last_of("/");
            if (requestTarget == route->path)
            {
                fullPath = route->getCgiPath();
                route->setCgiScriptName(findCGIScript(route->getCgiPath(), route->getCgiExtensions()));
                std::cout << "absence of scriptName, found: " << route->getCgiScriptName() << std::endl;
            }
            else
            {
                fullPath = route->getCgiPath() + requestTarget.substr(found);
                size_t queryPos = requestTarget.find("?");
                std::string scriptRAW = requestTarget.substr(0, queryPos);
                std::string scriptName = scriptRAW.substr(found + 1);
                if (scriptName.size())
                    route->setCgiScriptName(scriptRAW.substr(found + 1));
                else 
                    route->setCgiScriptName(findCGIScript(route->getCgiPath(), route->getCgiExtensions()));
            }
                
        }
        else 
        {
            std::cout << "requestTarget: " << requestTarget << std::endl;
            std::cout << "route->path: " << route->path << std::endl;
            if (requestTarget == route->path)
            {
                std::vector<std::string>::const_iterator it = findIndex(route->getRoot(), route->getIndex());
                if (it == route->getIndex().end())
                {
                    this->path = route->getRoot() + requestTarget;
                    return FORBIDDEN;
                }
                target = *it;
            }
            else
            {
                found = requestTarget.find(route->path);
                target = requestTarget.substr(found + route->path.size(), requestTarget.size());
                if (target[0] == '/')
                    target = target.substr(1, target.size());
            }
            fullPath = route->getRoot() + "/" + target;
        
        }
		found = fullPath.find_first_of("?");
        if (found != std::string::npos)
        {
            this->path   = fullPath.substr(0, found);
            this->query = fullPath.substr(found + 1);
            std::cout << "QUERY STRING: " << this->query << std::endl;
        }
        else
            this->path = fullPath;

		std::cout << "---------------- " << this->path << " ----------------" << std::endl;


		if (access(this->path.c_str(), F_OK) == -1)
            return NFOUND;
		return OK;
	}
	return BREQUEST;
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
	//this->body = new BodyRequest(); // TODO: handle delete
	this->body.type = NOTSET;

	// TODO: try to return if size 0
	if (this->method != POST)
		return OK;

	// Bad Request: content-length required
	if (this->headers.find("content-length") == this->headers.end())
		return BREQUEST;


	// TODO: be careful from overflow
	this->body.size = ConfigFile::toNumber(this->headers["content-length"]);
    
    //IF size > client-max-body-size then statusCode is 413 
    std::cout << "bodysize: " << this->body.size << std::endl;
    std::cout << "MaxclientSize: " << this->configServer->getClientMaxBodySize() << std::endl;

    if (this->body.size == 0)
        return BREQUEST;

    if (this->body.size > this->configServer->getClientMaxBodySize())
        return CTOOLARGE; 

    char body[this->body.size + 1];
    this->tokenizer.get(body, this->body.size + 1, '\0');
    body[this->body.size] = '\0';


	const std::string contentType = this->headers["content-type"];
	if (contentType == "application/x-www-form-urlencoded")
	{
		this->body.type = URLENCODED;
		Tokenizer t(body);
		while (!t.end())
		{
			// TODO: make a list of delimiters
			std::string key   = t.next("=");
			if (t.peek() == '=')
			{
				t.get();
			}
			std::string value = t.next("&");
			if (t.peek() == '&')
			{
				t.get();
			}
			// TODO: try to access by []
			this->body.urlencoded.insert(std::make_pair(key, value));
		}
	}
	else if (contentType == "text/plain")
	{
		this->body.type = RAW;
		this->body.raw = body;
	}
	else if (contentType.find("multipart/form-data") != std::string::npos)
	{
		this->body.type = MULTIPART;
		this->body.raw = body;
	}
	return OK;
}

StatusCode HttpRequest::parseHeaders(void)
{
	std::string key;
	std::string value;

	while (!tokenizer.end())
	{
		key = tokenizer.next(":\r\n");
		if (tokenizer.peek() == ':') tokenizer.get();
		HttpRequest::lower(key);
		tokenizer.trimSpace();
		value = tokenizer.next(HttpRequest::CRLF/*HttpRequest::delim*/);

		/*std::cout << "key  : " << key << std::endl;*/
		/*std::cout << "value: " << value << std::endl;*/

		if (key == "host")
		{
			if (value.empty() || this->headers.count(key) > 0) return BREQUEST;
			// NOTE: match host with server_names

			std::cout << "host: " << value << std::endl;

			this->headers[key] = tokenizer.next(HttpRequest::CRLF);
// 			if (!this->matchHost(value))
// 				return NFOUND;
		}
		if (key == "connection")
		{
			this->headers[key] = "keep-alive";
			
			if (value == "close")
			{
				this->headers[key] = value;//tokenizer.next(HttpRequest::CRLF);
			}
		}
		if (key == "content-length")
		{
			if (!ConfigFile::isNumber(value)) return BREQUEST;
// 			if (ConfigFile::toNumber(value) > this->configFile.getServers()[0].getClientBodySize())
// 			{
// 				return CTOOLARGE;
// 			}
			this->headers[key] = value;//tokenizer.next(HttpRequest::CRLF);
		}
		if (key == "content-type")
		{
			this->headers[key] = value;//tokenizer.next(";\r\n");
		}
		if (key == "user-agent")
		{
			this->headers[key] = value;//tokenizer.next(HttpRequest::CRLF);
		}

		if (this->tokenizer.isCRLF()) // checking "\r\n" and skip them
			if (this->tokenizer.isCRLF()) break;
	}
	// *NOTE: client must send one host header.
	if (this->headers.count("host") == 0) return BREQUEST;
	return OK;	
}

// TODO: try to put this function in configFile class
bool HttpRequest::matchHost(const std::string &host)
{
	this->serverName = host;
	size_t found = host.find(":");
	if (found != std::string::npos)
	{
        this->serverPort = host.substr(found + 1);
        this->serverName = host.substr(0, found);
    }
    std::vector<std::string>::iterator begin = this->configServer->getServerNames().begin();
    std::vector<std::string>::iterator end   = this->configServer->getServerNames().end();

	return std::find(begin, end, this->serverName) != end;
}

// TODO: what about CRLF at the EOF
//void	HttpRequest::parse(const char *buffer)
void	HttpRequest::parse(void)
{
	// TODO: how do i generate a HTTP response?

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
    
	if (this->statusCode == OK)
    {
	    this->statusCode = this->parseBody();
    }
	/* -----------   generate response  ----------- */

}

std::string	fileUpload(std::string const &body, std::string const &filename)
{
	///std::ofstream file()

	(void) body;

	std::cout << "body    : " << body << std::endl;
	std::cout << "filename: " << filename << std::endl;

	return std::string("HTTP/1.1 201 Created\r\n\r\n");	
}

std::string HttpRequest::DELETEmethod(const std::string &pathname)
{

    std::string statusLine = this->getStatusLine(NCONTENT);
    std::string headers = "Server: webserv/0.42\r\n";
    std::string body = HttpRequest::notAllowed("Allow: GET, POST, DELETE");
    headers += "Content-Length: 0\r\n";

    if ((access(pathname.c_str(), R_OK | W_OK) == -1))
        statusLine = this->getStatusLine(NALLOWED);
    else
        std::remove(pathname.c_str());
    
    headers += "Connection: keep-alive\r\n\r\n";
    return statusLine + headers + body + "\r\n";
}

std::string	HttpRequest::handler(Selector& selector, int clientFd)
{
	std::string cgiResponse;

	this->parse();
	Route *route = this->configServer->getRoute(this->_targetRequest);
    std::cout << "targetRequest: " << _targetRequest << std::endl;
    if (!route)
    {
        std::cout << __func__ << " in " << __FILE__ << ": route not found" << std::endl;
        this->statusCode = NFOUND;
    }
    if (this->statusCode == OK && route && route->isCgi())
    {
		CgiHandler *handler = new CgiHandler(this, route->getCgiScriptName(), route->getCgiPath());
		this->statusCode = handler->execute(selector, clientFd, this->_bodyPipe[0]);
        delete handler;
        if (this->statusCode == OK)
            return this->response;
    }
        std::cout << "Status code: " << this->statusCode << std::endl;

	switch (this->statusCode)
	{
		case BREQUEST: this->response = badRequest(); break;
		case NFOUND  : this->response = notFound();   break;
        case SERVERR : this->response = serverError(); break;
		case NALLOWED: this->response = notAllowed("Allow: GET, POST, DELETE"); break;
        case CTOOLARGE: this->response = payloadTooLarge(); break;
		case OK:
			switch (this->method)
			{
				case GET:    this->response = this->GETmethod(this->path, route);  break;
				case POST:   this->response = this->POSTmethod(this->path);    break;
				case DELETE: this->response = this->DELETEmethod(this->path); break;
				default:	 std::invalid_argument("NOT IMPLEMENTED - OTHER METHOD");
			}
			break;
		case FORBIDDEN:
			if (route->getAutoIndex() && this->method == GET)
				this->response = this->dirList(this->path);
			else
				this->response = forbidden();
			break;
		default: std::invalid_argument("NOT IMPLEMENTED - STATUS CODE");
	}
	return this->response;
}

std::string HttpRequest::dirList(std::string const &dirpath)
{
    struct dirent *dirnt;
	struct stat	  statbuf;
	DIR	*dir;
	char buff[100];

	

	std::string startLine = "HTTP/1.1 200 OK\r\n";
	std::string headers   = "Server: webserver/0.42\r\nContent-Type: text/html\r\n";

    std::string s = "Index of " + dirpath.substr(1, dirpath.size());
    std::string body = "<html>\n<head><title>" + s + "</title></head>\n";
    body += "<body>\n<h1>" + s + "</h1><hr><pre>";

    dir = opendir(dirpath.c_str());
    if (dir)
    {
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
				strftime(buff, sizeof(buff), "%d-%b-%y %H:%M ", &timestamp);
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

std::string HttpRequest::getStatusLine(StatusCode statusCode)
{
    std::map<StatusCode, std::string> statusPhrases;
    const std::string protocolHTTP = "HTTP/1.1";

    statusPhrases[OK] = "200 OK";
    statusPhrases[MVPERMANENT] = "301 Moved Permanently";
    statusPhrases[FOUND] = "302 Found";
	statusPhrases[BREQUEST] = "400 Bad Request";
	statusPhrases[FORBIDDEN] = "403 Forbidden";
	statusPhrases[NFOUND] = "404 Not Found";
	statusPhrases[NALLOWED] = "405 Not Allowed";
	statusPhrases[CTOOLARGE] = "413 Content Too Large";
    statusPhrases[SERVERR]  = "500 Internal Server Error";
	statusPhrases[NSUPPORTED] = "505 Not Supported";
    statusPhrases[NCONTENT] = "204 No Content";

    return protocolHTTP + " " + statusPhrases[statusCode] + "\r\n"; 
}

std::string	HttpRequest::GETmethod(std::string &pathname, Route *route)
{

    std::cout << pathname << std::endl;
    std::string statusLine  = getStatusLine(OK);
    std::string headers     = "Server: webserver/0.42\r\n";

    std::string body;
    if (route && route->getRedirection().size() != 0)
    {
        std::map<StatusCode, std::string>::iterator it = route->getRedirection().begin();
        statusLine = getStatusLine(it->first);
        if (it->first == MVPERMANENT || it->first == FOUND)
        {
            headers += "Location: " + it->second + "\r\n";
            pathname = "./err_pages/" + HttpRequest::toString(it->first) + ".html";
        }
        else
        {
            headers += "Content-Type: application/octet-stream\r\n"; 
            body = it->second;
        }
    }
    else
    {
        headers += "Content-Type: "   + HttpRequest::getMimeType(pathname) + "\r\n";
    }

    body = HttpRequest::readFile(pathname.c_str());
    headers += "Connection: " + this->headers["connection"] + "\r\n";
    headers += "Content-Length: " + HttpRequest::toString(body.size()) + "\r\n\r\n";
    return statusLine + headers + body;
}



std::string HttpRequest::POSTmethodRAW(const std::string &pathname)
{
	(void)pathname;
	return (this->body.raw);
}


std::string HttpRequest::POSTmethodURLENCODED(const std::string &pathname)
{
	(void)pathname;
	/*(void)cgiResponse;*/
	std::ostringstream bodyStream;
	for (std::map<std::string, std::string>::iterator it = this->body.urlencoded.begin();
			it != this->body.urlencoded.end(); ++it) {
		bodyStream << it->first << "=" << it->second << "\n";
	}
	return bodyStream.str();
}

std::string HttpRequest::createdFile(std::string filename)
{
	const std::string statusLine = "HTTP/1.1 201 Created\r\n";
	std::string headers = "Server: webserv/0.42\r\nContent-Type: text/html\r\n";
    headers += "\t\"filename\": " + filename + "\r\n";
	return statusLine + headers + "\r\n";
}

static size_t stringFound(const std::string& line, const std::string& toFind)
{
    if (toFind.empty() || line.size() < toFind.size())
        return std::string::npos; // Handle edge cases

    for (size_t pos = 0; pos <= line.size() - toFind.size(); ++pos) // Iterate within bounds
    {
        size_t i = 0;
        for (; i < toFind.size(); ++i)
        {
            if (line[pos + i] != toFind[i]) // Check for character match
                break;
        }
        if (i == toFind.size()) // If full match, return starting position
            return pos;
    }
    return std::string::npos; // No match found
}

static bool isBoundaryLine(const std::string& line, const std::string& boundary) 
{
    std::string finalBoundary = "--" + boundary + "--";
    return (stringFound(line, finalBoundary) != std::string::npos);
}

std::string HttpRequest::getBoundaryMultipart(std::stringstream& ss)
{
    std::string     line;
    std::string     contentTypeHeader = this->headers["content-type"];
    
    std::getline(ss, line);
    size_t boundaryStart = contentTypeHeader.find_last_of('-');
    std::string boundary = contentTypeHeader.substr(boundaryStart + 1);
    return boundary;

}

std::string HttpRequest::getFilenameMultipart(std::stringstream& ss)
{
    std::string     line;
    std::string     tmp = "filename=\"";

    std::getline(ss, line);
    size_t pos = line.find(tmp);
    std::string filename = line.substr(pos + tmp.size(), line.size() - (pos + tmp.size()) - 2);
    return filename;

}


std::string HttpRequest::getContentTypeMultipart(std::stringstream& ss)
{
    std::string     line;
    std::string     tmp = "Content-Type: ";

    std::getline(ss, line);
    size_t start = line.find(tmp);
    size_t end   = line.find('\r');
    start += tmp.size();
    std::string contentType = line.substr(start, end - start);
    return contentType;

}

std::string HttpRequest::createAbsoluteFilePath(const std::string& pathname, const std::string& filename)
{
    size_t pathEnd = pathname.find_last_of('/');
    std::string filePath = pathname.substr(0, pathEnd + 1);
    return (filePath + filename);
}



std::string HttpRequest::POSTmethodMULTIPART(const std::string& pathname)
{
    std::cout << "am i here" << std::endl;
    std::stringstream ss(this->body.raw);

    std::cout << "pathname: "<<pathname << std::endl;

    //getting boundary from line
    std::string boundary = this->getBoundaryMultipart(ss);
    std::cout << "boundary: '" << boundary <<"'" << std::endl;


    //getting fileName from line
    std::string filename = this->getFilenameMultipart(ss);
    std::cout << "filename: '" << filename << "'" << std::endl;

    //getting content-type from line
    std::string contentType = this->getContentTypeMultipart(ss);
    std::cout << "contentType: '" << contentType << "'" << std::endl;


    //skipping header line
    std::string line;
    std::getline(ss, line);

    //getting upload-folder path
    std::string uploadFilePathName = this->createAbsoluteFilePath(pathname, filename);
    std::cout << "uploadFilePathName: " <<uploadFilePathName << std::endl;

    
    
    //getting file contents
    std::ofstream output(uploadFilePathName.c_str(), std::ios::binary | std::ios_base::trunc);
    while (std::getline(ss, line)) 
    {
        if (line[0] == '\r') continue;
        if (!isBoundaryLine(line, boundary)) 
        {
            line += "\n";
            output.write(line.c_str(), line.size());
            std::cout << "Writing line: " << line;
            std::cout << "Line size: " << line.size() << std::endl;
        } 
        else 
        {
            break;
        }
    }

    // Close the output file
    output.close();

    //sending response with filename created 
    return HttpRequest::createdFile(filename);
}

std::string HttpRequest::POSTmethod(const std::string &pathname)
{
	std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers    = "Server: webserver/0.42\r\n";
    headers += "Content-Type: text/plain\r\n";

    std::string response;

	std::cout << "------------------------------------------------------------------------------------\n";
	switch (this->body.type)
	{
		case RAW:			response = POSTmethodRAW(pathname); break;
		case URLENCODED:	response = POSTmethodURLENCODED(pathname); break;
		case MULTIPART:		response = POSTmethodMULTIPART(pathname); break;
		default:			std::invalid_argument("NOT IMPLEMENTED - POST type not found"); break;
	}
	headers += "Content-Length: " + toString(response.size()) + "\r\n\r\n";
	std::cout << statusLine + headers + response;
	std::cout << "------------------------------------------------------------------------------------\n";

    return statusLine + headers + response;
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
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".xml"] = "application/xml";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".svg"] = "image/svg+xml";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".zip"] = "application/zip";
    mimeTypes[".tar"] = "application/x-tar";
    mimeTypes[".gz"] = "application/gzip";
    mimeTypes[".mp3"] = "audio/mpeg";
    mimeTypes[".wav"] = "audio/wav";
    mimeTypes[".mp4"] = "video/mp4";
    mimeTypes[".avi"] = "video/x-msvideo";
    mimeTypes[".mov"] = "video/quicktime";
    mimeTypes[".bin"] = "application/octet-stream";

    // Find the file extension
    size_t dotPos = file.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream"; // Default MIME type for unknown files
    }

    std::string extension = file.substr(dotPos);

    // Lookup the extension in the MIME types map
    std::map<std::string, std::string>::iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) 
        return it->second;
    return "text/html"; // Default MIME type for unknown files
    //return "application/octet-stream"; // Default MIME type for unknown files
}

std::string execCGI(void)
{
	std::string response;
	// TODO: unchunked data
	throw std::invalid_argument("TODO: " + std::string(__FUNCTION__));
	return response;
}
