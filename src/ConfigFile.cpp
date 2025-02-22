#include "ConfigFile.hpp"
#include <iostream>

#define TODO() throw std::invalid_argument(__FUNCTION__)

Tokenizer ConfigFile::tokenizer;

void	error(std::string const &errMsg)
{
	std::cerr << "error: " << errMsg << std::endl;
	exit(1);
}

/* ------------------------- ConfigFile ------------------------- */
/* ------------- Constructors --------------- */

const char *ConfigFile::delim = "#{};\f\n\r\t\v ";
const char *ConfigFile::pathname = "./webserv.conf";

ConfigFile::ConfigFile(void) {}

ConfigFile::~ConfigFile(void) {}

ConfigFile::ConfigFile(const char *pathname)
{
	file.open(pathname, std::ios::in);
	if (!file.is_open())
	{
		perror(pathname);
		exit(1);
	}
	this->tokenizer << this->file.rdbuf();
	this->parse();
	file.close();
}

ConfigFile::ConfigFile(ConfigFile const &other)
{
	ConfigFile::operator=(other);
}

ConfigFile	&ConfigFile::operator=(ConfigFile const &other)
{
	if (this != &other)
	{

		this->servers = other.servers;
	}
	return *this;
}


/* ------------- Methods --------------- */

void	ConfigFile::parse(void)
{
	
	while (!this->tokenizer.end())
	{
		ConfigServer server;
		this->tokenizer.trim();

		if (this->tokenizer.peek() == '#')
		{
			this->tokenizer.consume();
			continue;
		}
		std::string token = this->tokenizer.next(ConfigFile::delim);
		if (token.empty()) continue;
		if (token == "server")
			server.parse();
		else
			error("unexpected token");
		this->servers.push_back(server);
	}
}

std::vector<ConfigServer>	&ConfigFile::getServersConfig(void)
{
	return this->servers;
}

/* ------------- Static Methods --------------- */

bool	ConfigFile::isNumber(std::string const &str)
{
	if (str.empty()) return false;
	size_t count = std::count_if(str.begin(), str.end(), isdigit);
	return count == str.size();
}

size_t	ConfigFile::toNumber(std::string const &str)
{
	std::stringstream ss(str);
	size_t num;

	ss >> num;
	return num;
}

Tokenizer	&ConfigFile::getTokenizer(void)
{
  return tokenizer;
}

Method ConfigFile::isMethod(std::string const &method)
{
	const std::string methods[] = {"GET", "POST", "DELETE"};

	for (size_t i = 0; i < 3; i += 1)
	{
		if (methods[i] == method) return static_cast<Method>(i);
	}
	return static_cast<Method>(-1);
}


/* ------------------------- ConfigServer ------------------------- */

/* ------------- Constructors --------------- */

ConfigServer::ConfigServer(void)
{
	this->isResized = false;
	this->client_max_body_size = 1024;
	this->tokenizer = &ConfigFile::getTokenizer();
}

ConfigServer::~ConfigServer(void) {}

ConfigServer::ConfigServer(ConfigServer const &other)
{
	ConfigServer::operator=(other);
}

ConfigServer &ConfigServer::operator=(ConfigServer const &other)
{
	if (this != &other)
	{
        this->ports = other.ports;
		this->client_max_body_size = other.client_max_body_size;
		this->root = other.root;
		this->server_names = other.server_names;
		this->index = other.index;
		this->error_pages = other.error_pages;
		this->routes = other.routes;
        this->redirection = other.redirection;
		this->tokenizer = other.tokenizer;
	}
	return *this;
}

/* ------------- Methods --------------- */
void	ConfigServer::parseListen(void)
{
	int	port;
	std::string token;

	while (!this->tokenizer->end())
	{
		this->tokenizer->trim();
		if (this->tokenizer->peek() == '#')
		{
			this->tokenizer->consume();
		}
		if (std::string("{}\n;").find(this->tokenizer->peek()) != std::string::npos)
			break;

		token = this->tokenizer->next(ConfigFile::delim);
		if (token.empty())
		{
			error("invalid argument");
		}
		if (!ConfigFile::isNumber(token))
		{
			error("invalid port number: " + token);
		}

		port = ConfigFile::toNumber(token);

		if (port > 65535)
		{
			error("port " + token + " must be between 0 - 65535");
		}
		this->ports.push_back(port);
	}

	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

void print(std::string e)
{
	std::cout << "-> " << e << std::endl;
}

void	ConfigServer::parseIndex(void)
{
	char c;
	std::string token;


    //TODO: why do you have an index.resize(1) @ahmed from @g
	if (!this->isResized)
	{
	//	this->index.resize(1);
		this->isResized = true;
	}

	this->tokenizer->trim();
	while (!this->tokenizer->end())
	{
		this->tokenizer->trim();
		c = this->tokenizer->peek();
		if (std::string("{}\n;").find(c) != std::string::npos)
			break;

		// TODO: think about it
		if (c == '#')
		{
			this->tokenizer->consume();
			continue;
		}
		token = this->tokenizer->next(ConfigFile::delim);
		if (std::find(index.begin(), index.end(), token) == index.end())
        {
            if (token.size() != 0)
                this->index.push_back(token);
        }
	}
	if (this->index.size() == 0)
	{
		error("index: invalid argument");
	}
	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

void	ConfigServer::parseServerName(std::vector<std::string> &vec)
{
	char c;
	std::string token;

	this->tokenizer->trim();
	while (!this->tokenizer->end())
	{
		this->tokenizer->trim();
		c = this->tokenizer->peek();
		if (std::string("{}\n;").find(c) != std::string::npos)
			break;

		// TODO: think about it
		if (c == '#')
		{
			this->tokenizer->consume();
			continue;
		}
		vec.push_back(this->tokenizer->next(ConfigFile::delim));
	}
	
	if (vec.size() == 0)
	{
		error("server_name: invalid argument");
	}
	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

void	ConfigServer::parseRoot(void)
{
	this->tokenizer->trim();
	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}
	this->root = this->tokenizer->next(ConfigFile::delim);
	if (this->root.empty())
	{
		error("root: invalid argument");
	}

	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

void	ConfigServer::parseErrorPage(void)
{
	std::string token;

	this->tokenizer->trim();

	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}

	token = this->tokenizer->next(ConfigFile::delim);

	if (!ConfigFile::isNumber(token))
	{
		error("status code must be a number");
	}

	this->tokenizer->trim();

	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}

	int	statusCode = ConfigFile::toNumber(token);
	if (statusCode < 300 || statusCode > 600)
	{
		error("`" + token + "` must be between 300 and 599");
	}
    StatusCode code = static_cast<StatusCode>(ConfigFile::toNumber(token));

	this->tokenizer->trim();
	token = this->tokenizer->next(ConfigFile::delim);
	if (token.empty())
	{
		error("error_page: invalid argument");
	}
	this->error_pages[code] = token;
	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

void	ConfigServer::parseBodySize(void)
{
	std::string token;

	this->tokenizer->trim();
	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}
	token = this->tokenizer->next(ConfigFile::delim);
	if (token.empty())
	{
		error("client_max_body_size: invalid argument");
	}
	if (!ConfigFile::isNumber(token))
	{
		error("invalid value: " + token);
	}

	this->client_max_body_size = ConfigFile::toNumber(token);


	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

typedef void	(Route::*RouteFuncPtr)();
void	ConfigServer::parseRoute(void)
{
	Route route;
    route.root  = this->root;
    route.index = this->index;
    route.redirection = this->redirection;
    route.error_pages = this->error_pages;

	this->tokenizer->trim();
	if (this->tokenizer->peek() == '#')	this->tokenizer->consume();

	std::string _path = this->tokenizer->next(ConfigFile::delim);
	route.path = _path;

	if (_path.empty())	error("location: invalid argument");

	this->tokenizer->trim();
	this->tokenizer->expected('{', ConfigFile::delim);

	std::string token;
	std::map<std::string, RouteFuncPtr> fptrs;

	fptrs["allow_method"] = &Route::parseMethods;
	fptrs["return"] = &Route::parseRedirection;
	fptrs["alias"] = &Route::parseRoot;
	fptrs["autoindex"] = &Route::parseAutoIndex;
	fptrs["index"] = &Route::parseIndex;
	fptrs["cgi_path"] = &Route::parseCgiPath;
	fptrs["cgi_ext"] = &Route::parseCgiExtensions;
	std::map<std::string, RouteFuncPtr>::iterator it;
	while (!this->tokenizer->end() && this->tokenizer->peek() != '}')
	{
		this->tokenizer->trim();
		if (this->tokenizer->peek() == '#')
		{
			this->tokenizer->consume();
			continue;
		}
		token = this->tokenizer->next(ConfigFile::delim);
		if (token.empty()) continue;
		it = fptrs.find(token);
		if (it != fptrs.end())
			(route.*it->second)();
		else
			error("invalid keyword: " + token);
	}
	this->routes.push_back(route);
	this->tokenizer->trim();
	this->tokenizer->expected('}', ConfigFile::delim);
}

void	ConfigServer::parse(void)
{
	std::string token;

	this->tokenizer->trim();
	this->tokenizer->expected('{', ConfigFile::delim);
	
	while (!this->tokenizer->end() && this->tokenizer->peek() != '}')
	{
		this->tokenizer->trim();
		if (this->tokenizer->peek() == '#')
		{
			this->tokenizer->consume();
			continue;
		}

		token = this->tokenizer->next(ConfigFile::delim);

		if (token == "listen")
		{
			this->parseListen();
		}
		else if (token == "server_name")
		{
			this->parseServerName(this->server_names);
		}
		else if (token == "index")
		{
			this->parseIndex();
		}
		else if (token == "alias")
		{
			this->parseRoot();
		}
		else if (token == "error_page")
		{
			this->parseErrorPage();
		}
		else if (token == "client_max_body_size")
		{
			this->parseBodySize();
		}
		else if (token == "location")
		{
			this->parseRoute();
		}
		else
		{
			error("invalid keyword: " + token);
		}

		this->tokenizer->trim();
	}

	this->tokenizer->trim();
	this->tokenizer->expected('}', ConfigFile::delim);
}

void	ConfigServer::clear(void)
{

}

/* ------------- Getters --------------- */

std::vector<uint16_t>	&ConfigServer::getPorts(void)
{
	return this->ports;
}

size_t	ConfigServer::getClientMaxBodySize(void)
{
	return this->client_max_body_size;
}
std::string	&ConfigServer::getRoot(void)
{
	return this->root;
}
std::vector<std::string> &ConfigServer::getServerNames(void)
{
	return this->server_names;
}
std::vector<std::string> &ConfigServer::getIndex(void)
{
    /*for (unsigned int i = 0; i < this->index.size(); i += 1)*/
    /*    std::cout << "getIndex: " << this->index[i] << std::endl;*/
    return this->index;
}
std::map<StatusCode, std::string> &ConfigServer::getErrorPages(void)
{
	return this->error_pages;
}

Route	*ConfigServer::getRoute(std::string const &path)
{
	Route *route = NULL;
	for (size_t i = 0; i < routes.size(); i += 1)
	{
		if (path.find(routes[i].path.c_str(), 0, routes[i].path.size()) != std::string::npos)
			route = &routes[i];
	}
	return route;
}

std::vector<Route>	&ConfigServer::getRoutes(void)
{
	return this->routes;
}


/* ------------------------- Route ------------------------- */

/* ------------- Constructors --------------- */

Route::Route(void)
{
	this->isResized = false;
	this->autoindex = false; // default one
	this->tokenizer = &ConfigFile::getTokenizer();
}

Route::~Route(void) {}

Route::Route(Route const &other) : ConfigServer(other)
{
	Route::operator=(other);
}

Route	&Route::operator=(Route const &other)
{
	if (this != &other)
	{
        this->redirection = other.redirection;
		this->path    = other.path;
		this->methods = other.methods;
		this->root = other.root;
		this->autoindex = other.autoindex;
		this->tokenizer = other.tokenizer;
		this->index = other.index;
		this->cgiPath = other.cgiPath;
		this->cgiExtensions = other.cgiExtensions;
	}
	return *this;
}

/* ------------- Methods --------------- */

void	Route::parseMethods(void)
{
	char c;
	std::string token;

	this->tokenizer->trim();

	while (!this->tokenizer->end())
	{
		c = this->tokenizer->peek();
		if (c == ';' || c == '\n')
			break;

		if (c == '#')
		{
			this->tokenizer->consume();
			continue;
		}

		token = this->tokenizer->next(ConfigFile::delim);
		Method method = ConfigFile::isMethod(token);
		if (static_cast<int>(method) == -1)
		{
			error("allow_method invalid: " + token);
		}
		this->methods.push_back(method);
		this->tokenizer->trim();
	}

	if (methods.size() == 0)
	{
		error("allow_method: invalid argument");
	}

	this->tokenizer->expected(';', ConfigFile::delim);
}

void	Route::parseRedirection(void)
{
    this->redirection.clear();
    std::string key, value;

	this->tokenizer->trim();

	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}

	key = this->tokenizer->next(ConfigFile::delim);

	if (!ConfigFile::isNumber(key))
	{
		error("status code must be a number");
	}

	this->tokenizer->trim();

	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}

	//int	statusCode = ConfigFile::toNumber(token);
// 	if (statusCode < 300 || statusCode > 600)
// 	{
// 		error("`" + token + "` must be between 300 and 599");
// 	}

	this->tokenizer->trim();
	value = this->tokenizer->next(ConfigFile::delim);
	if (value.empty())
	{
		error("return: invalid argument");
	}

    StatusCode statusCode = static_cast<StatusCode>(ConfigFile::toNumber(key));
    this->redirection[statusCode] = value;

	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);


    std::cout << "route.path: " <<  this->path << std::endl;
    std::cout << "redi key   : " << this->redirection.begin()->first << std::endl;
    std::cout << "redi value : " << this->redirection.begin()->first << std::endl;
}

void	Route::parseAutoIndex(void)
{
	std::string token;

	this->tokenizer->trim();
	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}
	token = this->tokenizer->next(ConfigFile::delim);
	if (token.empty())
	{
		error("autoindex: invalid argument");
	}
	if (token == "on")
	{
		this->autoindex = true;
	}
	else if (token == "off")
	{
		this->autoindex = false;
	}
	else
	{
		error("autoindex: invalid value " + token);
	}

	this->tokenizer->expected(';', ConfigFile::delim);
}

void    Route::parseCgiPath(void)
{
	this->tokenizer->trim();
	if (this->tokenizer->peek() == '#') this->tokenizer->consume();
	this->cgiPath = this->tokenizer->next(ConfigFile::delim);

	if (this->cgiPath.empty()) error("cgi_path: invalid argument");
	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

void Route::setCgiScriptName(std::string scriptName)
{
    this->cgiScriptName = scriptName;
}

bool Route::isCgi(void)
{
	return !this->cgiPath.empty() && !this->cgiExtensions.empty();
}

void    Route::parseCgiExtensions(void)
{
	char c;
	std::string token;

	this->tokenizer->trim();
	while (!this->tokenizer->end())
	{
		this->tokenizer->trim();
		c = this->tokenizer->peek();
		if (std::string("{}\n;").find(c) != std::string::npos)
			break;
		if (c == '#')
		{
			this->tokenizer->consume();
			continue;
		}
		cgiExtensions.push_back(this->tokenizer->next(ConfigFile::delim));
	}
	if (cgiExtensions.size() == 0) error("cgi_ext: invalid argument");
	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

/* ------------- Getters --------------- */

bool				Route::getAutoIndex(void)
{
	return this->autoindex;
}

std::string 					&Route::getRoot(void)
{
	return this->root;
}
std::vector<Method>				  &Route::getMethods(void)
{
	return this->methods;
}
std::map<StatusCode, std::string> &Route::getRedirection(void)
{
	return this->redirection;
}

std::string Route::getCgiPath(void)
{
	return this->cgiPath;
}

std::string Route::getCgiScriptName(void)
{
	return this->cgiScriptName;
}

std::vector<std::string> &Route::getCgiExtensions(void)
{
	return this->cgiExtensions;
}

template<typename T>
void	print(const T &e)
{
	std::cout << e << " ";
}

std::ostream	&operator<<(std::ostream &os, ConfigServer &obj)
{
	std::cout << "ports: ";
	std::for_each(obj.getPorts().begin(), obj.getPorts().end(), print<uint16_t>);
	std::cout << std::endl;
	
	std::cout << "server_names: ";
	std::for_each(obj.getServerNames().begin(), obj.getServerNames().end(), print<std::string>);
	std::cout << std::endl;

	std::cout << "index: ";
	std::for_each(obj.getIndex().begin(), obj.getIndex().end(), print<std::string>);
	std::cout << std::endl;

	std::cout << "root: " << obj.getRoot() << std::endl;

	std::map<StatusCode, std::string> errs(obj.getErrorPages());
	if (errs.size() > 0)
	{
		std::cout << "error_page: " << errs.begin()->first << ", " << errs.begin()->second << std::endl;
	}

	std::cout << "client_max_body_size: " << obj.getClientMaxBodySize() << std::endl;
	

	std::vector<Route> routes(obj.getRoutes());
	for (size_t i = 0; i < routes.size(); i += 1)
	{
		std::cout << "location " << routes[i].path << std::endl;;

		std::cout << "    index: ";
		std::for_each(routes[i].getIndex().begin(), routes[i].getIndex().end(), print<std::string>);
		std::cout << std::endl;

		std::cout << "    autoindex: " << (routes[i].getAutoIndex() ? "on" : "off") << std::endl;

		std::cout << "    root: " << routes[i].getRoot() << std::endl;
	}
	return os;
}
