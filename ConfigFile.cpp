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
#define PUT(str, str2) std::cout << str << "`" << str2 << "`" << std::endl;


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
	ConfigServer server;
	
	while (!this->tokenizer.end())
	{
		this->tokenizer.trim();

		if (this->tokenizer.peek() == '#')
		{
			this->tokenizer.consume();
			continue;
		}

		std::string token = this->tokenizer.next(ConfigFile::delim);
		if (token.empty()) continue;
		std::cout << "`" << token << "`" << std::endl;
		if (token == "server")
		{
			server.parse();
		}
		else
		{
			error("unexpected token");
		}
	}
	this->servers.push_back(server);
}

std::vector<ConfigServer>	&ConfigFile::getServers(void)
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
	this->index.push_back("index.html");
	this->port      = 80; // default one
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
		this->port = other.port;
		this->client_max_body_size = other.client_max_body_size;
		this->root = other.root;
		this->server_names = other.server_names;
		this->index = other.index;
		this->error_pages = other.error_pages;
		this->routes = other.routes;
		this->tokenizer = other.tokenizer;
	}
	return *this;
}

/* ------------- Methods --------------- */
void	ConfigServer::parseListen(void)
{
	// TODO: try to parse multiple ports
	std::string token;

	this->tokenizer->trim();

	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}
	token = this->tokenizer->next(ConfigFile::delim);
	if (token.empty())
	{
		error("invalid argument");
	}
	if (!ConfigFile::isNumber(token))
	{
		error("invalid port number: " + token);
	}
	this->port = ConfigFile::toNumber(token);
	
	if (this->port > 65535)
	{
		error("port " + token + " must be between 0 - 65535");
	}
	this->tokenizer->trim();
	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
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


	if (!this->isResized)
	{
		this->index.resize(1);
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
			this->index.push_back(token);
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
	
	std::for_each(vec.begin(), vec.end(), print);
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

	this->tokenizer->trim();
	token = this->tokenizer->next(ConfigFile::delim);
	if (token.empty())
	{
		error("error_page: invalid argument");
	}

	this->error_pages[static_cast<StatusCode>(ConfigFile::toNumber(token))] = token;

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

void	ConfigServer::parseRoute(void)
{
	Route route;

	{
		route.root  = this->root;
		route.index = this->index;
	}

	this->tokenizer->trim();

	if (this->tokenizer->peek() == '#')
	{
		this->tokenizer->consume();
	}

	std::string _path = this->tokenizer->next(ConfigFile::delim);
	route.path = _path;

	if (_path.empty())
	{
		error("location: invalid argument");
	}

	this->tokenizer->trim();
	this->tokenizer->expected('{', ConfigFile::delim);

	std::string token;

	while (!this->tokenizer->end() && this->tokenizer->peek() != '}')
	{
		this->tokenizer->trim();
		if (this->tokenizer->peek() == '#')
		{
			this->tokenizer->consume();
			continue;
		}

		token = this->tokenizer->next(ConfigFile::delim);

		if (token == "allow_method")
		{
			route.parseMethods();
		}
		if (token == "return")
		{
			route.parseRedirection();
		}
		if (token == "root")
		{
			route.parseRoot();
		}
		if (token == "autoindex")
		{
			route.parseAutoIndex();
		}
		if (token == "index")
		{
			//route.parseServerName(route.index);
			route.parseIndex();
			std::cout << "index of Route:\n";
			std::for_each(route.index.begin(), route.index.end(), print);
			std::cout << "index of Server:\n";

			
		}
		//	break;
	}

	this->routes.push_back(route);
	//this->routes[path] = route;

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
			std::cout << "index of Server:" << std::endl;
			std::for_each(index.begin(), index.end(), print);
			//this->parseServerName(this->index);
		}
		else if (token == "root")
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

/* ------------- Getters --------------- */

size_t	ConfigServer::getPort(void)
{
	return this->port;
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
		{
			route = &routes[i];
		}
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
		this->path    = other.path;
		this->methods = other.methods;
		this->root = other.root;

		this->autoindex = other.autoindex;
		this->tokenizer = other.tokenizer;
		this->index = other.index;
		//this->index   = other.index;
		//this->indices = other.indices;
		//this->redirection = other.redirection;
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

	//int	statusCode = ConfigFile::toNumber(token);
// 	if (statusCode < 300 || statusCode > 600)
// 	{
// 		error("`" + token + "` must be between 300 and 599");
// 	}

	this->tokenizer->trim();
	token = this->tokenizer->next(ConfigFile::delim);
	if (token.empty())
	{
		error("return: invalid argument");
	}

	this->redirection[static_cast<StatusCode>(ConfigFile::toNumber(token))] = token;
	this->tokenizer->trim();
	this->tokenizer->expected(';', ConfigFile::delim);
}

// void	Route::parseRoot(void)
// {
// 	this->tokenizer->trim();
// 	if (this->tokenizer->peek() == '#')
// 	{
// 		this->tokenizer->consume();
// 	}
// 	this->root = this->tokenizer->next(ConfigFile::delim);
// 	if (this->root.empty())
// 	{
// 		error("root: invalid argument");
// 	}
// 	this->tokenizer->trim();
// 	this->tokenizer->expected(';', ConfigFile::delim);
// }

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

// void	Route::parseIndex(void)
// {
// 	int	c;
// 	this->tokenizer->trim();
// 	while (!this->tokenizer->end())
// 	{
// 		c = this->tokenizer->peek();
// 		if (std::string("{}\n;").find(c) != std::string::npos)
// 			break;
// 		if (c == '#')
// 		{
// 			this->tokenizer->consume();
// 			continue;
// 		}
// 		this->index.push_back(this->tokenizer->next(ConfigFile::delim));
// 	}
// 	if (this->index.size() == 0) error("index: invalid argument");
// 	
// 	// TODO: add default one

// 	this->tokenizer->expected(';', ConfigFile::delim);
// }

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

// std::vector<std::string>	&Route::getIndices(void)
// {
// 	return this->index;
// }

std::ostream	&operator<<(std::ostream &os, ConfigFile &obj)
{
	std::vector<ConfigServer> servers = obj.getServers();
	for (size_t i = 0; i < servers.size(); i += 1)
	{
		PUT("listen     : ", servers[i].getPort());
		PUT("server_name: ", servers[i].getServerNames()[0]);
		std::vector<Route> routes = servers[i].getRoutes();
		for (size_t i = 0; i < routes.size(); i += 1)
		{
			PUT("location: ", routes[i].path);
			PUT("size of index: ", routes[i].getIndex().size());
			//PUT("index: ", routes[i].getIndices()[0]);
		}
	}
	std::cout << std::endl;
	return os;
}
