#include "ConfigFile.hpp"

/* ---------------------- route -------------------- */

Route::Route(void)
{
	this->autoIndex = false;
}

bool Route::getAutoIndex(void)
{
	return this->autoIndex;
}

const std::string &Route::getPath(void) const
{
	return this->path;
}
const std::string &Route::getRoot(void) const
{
	return this->root;
}
std::vector<Method> &Route::getMethods(void)
{
	return this->methods;
}
std::vector<std::string> &Route::getDefaultFiles(void)
{
	return this->default_files;
}
	
std::map<StatusCode, std::string>	&Route::getRedirection(void)
{
	return this->redirection;
}


void	Route::parseMethods(const std::string &line)
{
	std::stringstream ss(line);
	std::string token;
	Method method;

	while (!ss.eof())
	{
		token  = ConfigFile::getToken(ss);
		method = ConfigFile::isMethod(token);
		if (static_cast<int>(method) == -1)
		{
			std::cerr << "[ERROR]: invalid method: " << token << std::endl;
			exit(1);
		}
		this->methods.push_back(method);
	}
}

void	Route::parseDefaultFile(const std::string &line)
{
	if (line.empty())
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}
	std::stringstream ss(line);
	while (!ss.eof())
	{
		this->default_files.push_back(ConfigFile::getToken(ss));
	}
}

void	Route::parseAutoIndex(const std::string &line)
{
	std::stringstream ss(line);
	std::string token = ConfigFile::getToken(ss);

	if (token.empty() || !ConfigFile::getToken(ss).empty())
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}


	if (token == "on")
		this->autoIndex = true;
	else if (token == "false")
	{
		this->autoIndex = false;
	}
	else
	{
		std::cerr << "[ERROR]: invalid value: " << token << std::endl;
		exit(1);
	}
}

void	Route::parseRoot(const std::string &line)
{	
	std::stringstream ss(line);
	this->root = ConfigFile::getToken(ss);

	if (this->root.empty() || !ConfigFile::getToken(ss).empty())
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}
}

void	Route::parseRedirection(const std::string &line)
{
	std::stringstream ss(line);
	std::string token = ConfigFile::getToken(ss);
	StatusCode statusCode;

	if (!ConfigFile::isNumber(token))
	{
		std::cerr << "[ERROR]: invalid number: " << token << std::endl;
		exit(1);
	}
	statusCode = static_cast<StatusCode>(ConfigFile::toNumber(token));
	token      = ConfigFile::getToken(ss);
	if (token.empty())
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}
	// TODO: check range of status code
	if (this->redirection.find(statusCode) == this->redirection.end())
	{
		this->redirection[statusCode] = token;
	}
}


void	Route::setPath(const std::string &path)
{
	this->path = path;
}

/* ---------------------- ConfigServer -------------------- */

std::vector<Route>	&ConfigServer::getRoutes(void)
{
	return this->routes;
}

std::vector<std::string> &ConfigServer::getServerNames(void)
{
	return this->serverNames;
}
std::vector<uint16_t>		 &ConfigServer::getPorts(void)
{
	return this->ports;
}
std::map<StatusCode, std::string> &ConfigServer::getErrorPages(void)
{
	return this->error_pages;
}
size_t ConfigServer::getClientBodySize()
{
	return this->clientBodySize;
}

void	ConfigServer::parseRoute(std::string path, std::stringstream &ss)
{
	bool isRoute = false;
	Route route;
	std::string line;
	std::string token;

	route.setPath(path);
	while (!ss.eof() && !isRoute)
	{
		size_t prev = ss.tellg();
		line = ConfigFile::getLine(ss);
		std::stringstream ss2(ConfigFile::cutComment(line));
		while (!ss2.eof())
		{
			token = ConfigFile::getToken(ss2);
			if (token.empty()) continue ;
			if (token == "route" || token == "server:")
			{
				ss.seekg(prev);
				isRoute = true;
				break;
			}
			if (token == "methods")
			{
				route.parseMethods(ConfigFile::getLine(ss2));
			}
			else if (token == "return")
			{
				route.parseRedirection(ConfigFile::getLine(ss2));	
			}
			else if (token == "root")
			{
				route.parseRoot(ConfigFile::getLine(ss2));
			}
			else if (token == "autoindex")
			{
				route.parseAutoIndex(ConfigFile::getLine(ss2));
			}
			else if (token == "default_file")
			{
				route.parseDefaultFile(ConfigFile::getLine(ss2));
			}
			else
			{
				std::cerr << "[ERROR]: invalid route's keyword: " << token << std::endl;
				exit(1);
			}

			// add more for CGI
		}
	}
	this->routes.push_back(route);
}

std::string ConfigServer::parseRoutePath(std::stringstream &ss)
{
	std::string token = ConfigFile::getToken(ss);

	if (token.empty())
	{
		std::cerr << "[ERROR]: route's should not be empty" << std::endl;
		exit(1);
	}
	if (!ss.eof())
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}
	return token;
}

void	ConfigServer::parseClientBodySize(const std::string &line)
{
	// TODO: check if line is empty
	std::stringstream ss(line);
	std::string token = ConfigFile::getToken(ss);
	if (!ConfigFile::isNumber(token))
	{
		std::cerr << "[ERROR]: invalid number: " << token << std::endl;
		exit(1);
	}
	this->clientBodySize = ConfigFile::toNumber(token);
}

void	ConfigServer::parseErrorPages(const std::string &line)
{
	std::stringstream ss(line);
	StatusCode statusCode;
	std::string token = ConfigFile::getToken(ss);

	if (!ConfigFile::isNumber(token))
	{
		std::cerr << "[ERROR]: invalid status code: " << token << std::endl;
		exit(1);
	}
	// TODO: check range of status code
	statusCode = static_cast<StatusCode>(ConfigFile::toNumber(token));
	if (this->error_pages.find(statusCode) == this->error_pages.end())
	{
		this->error_pages[statusCode] = ConfigFile::getToken(ss);
	}
	if (!ss.eof())
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}
}

void	ConfigServer::parsePorts(const std::string &line)
{
	std::string token;
	std::stringstream ss(line);
	int	port;

	while (!ss.eof())
	{
		token = ConfigFile::getToken(ss);
		if (token.empty()) continue;
		if (!ConfigFile::isNumber(token))
		{
			std::cerr << "[ERROR]: invalid port number: " << token << std::endl;
			exit(1);
		}
		port = ConfigFile::toNumber(token);
		if (port < PORT_MIN || port > PORT_MAX)
		{
			std::cerr << "[ERROR]: port is out of range: " << token << std::endl;
			exit(1);
		}
		this->ports.push_back(port);
	}
	if (this->ports.size() == 0)
	{
		std::cerr << "[ERROR]: invalid arguments" << std::endl;
		exit(1);
	}
}

void	ConfigServer::parseServerNames(const std::string &line)
{
	std::string token;
	std::stringstream ss(line);
	while (!ss.eof())
	{
		token = ConfigFile::getToken(ss);
		this->serverNames.push_back(token);
	}
	if (this->serverNames.size() == 0)
	{
		std::cerr << "[ERROR]: server_name needs at least one argument" << std::endl;
		exit(1);
	}
}


ConfigServer::ConfigServer(void)
{
	this->clientBodySize = 1024;
}

/* ---------------------- ConfigFile -------------------- */

void	ConfigFile::printTree(void)
{
	std::vector<ConfigServer>::iterator it = this->servers.begin();
	std::vector<Route>::iterator rit;
	std::vector<Route> routes;


	while (it != this->servers.end())
	{
		std::cout << "--------------------------------" << std::endl;
		std::cout << "ports:" << std::endl;
		::iter(it->getPorts(), print<int>);
		std::cout << "server_names:" << std::endl;
		::iter(it->getServerNames(), print<std::string>);
		std::cout << "client_body_size:\n    " << it->getClientBodySize() << std::endl;
		std::cout << "error pages:" << std::endl;
		::iter(it->getErrorPages(), printKeyValue<std::pair<StatusCode, std::string> >);

		routes = it->getRoutes();

		for (rit = routes.begin(); rit != routes.end(); rit++)
		{
			std::cout << "Route: " << rit->getPath() << std::endl;
			std::cout << "    root: " << rit->getRoot() << std::endl;
			std::cout << "    methods:" << std::endl;
			::iter(rit->getMethods(), print<Method>);
			std::cout << "    redirections:" << std::endl;
			::iter(rit->getRedirection(), printKeyValue<std::pair<StatusCode, std::string> >);
			std::cout << "    autoindex: " << (rit->getAutoIndex() ? "on" : "off") << std::endl;
		}
		it++;
		std::cout << "--------------------------------" << std::endl;
	}

}

std::vector<ConfigServer>	&ConfigFile::getServers(void)
{
	return this->servers;
}

void	ConfigFile::parseServer(void)
{
	bool isServer = false;
	ConfigServer server;
	std::string line;
	std::string token;
	//ConfigServer server;
	while (!this->ss.eof() && !isServer)
	{
		size_t prev = ss.tellg();
		line = getLine(this->ss);
		// TODO: think why are you checking spaces?
		if (isSpace(line)) continue;
		std::stringstream ss2(ConfigFile::cutComment(line));
		while (!ss2.eof())
		{
			token = getToken(ss2);
			
			if (token == "server:")
			{
				ss.seekg(prev);
				isServer = true;
				break;
			}

			if (token.empty()) continue;

			if (token == "server_name")
			{
				server.parseServerNames(getLine(ss2));
			}
			else if (token == "listen")
			{
				server.parsePorts(getLine(ss2));
			}
			else if (token == "error_page")
			{
				server.parseErrorPages(getLine(ss2));
			}
			else if (token == "client_body_size")
			{
				server.parseClientBodySize(getLine(ss2));
			}
			else if (token == "route")
			{
				server.parseRoute(server.parseRoutePath(ss2), ss);
			}
			else
			{
				std::cerr << "[ERROR]: invalid keyword: " << token << std::endl;
				exit(1);
			}
		}
	}
	this->servers.push_back(server);
}

std::string ConfigFile::cutComment(std::string &line)
{
	size_t found = line.find_first_of("#");
	if (found != std::string::npos)
	{
		line.erase(found, line.size());
	}
	return line;
}


ConfigFile::ConfigFile(const char *pathname)
{
	std::string line;

	file.open(pathname);
	if (!file.is_open())
	{
		std::cerr << "[ERROR]: " << strerror(errno) << ": " << pathname << std::endl;
		exit(1);
	}
	this->ss << file.rdbuf();
	while (!this->ss.eof())
	{
		line = getLine(this->ss);
		ConfigFile::cutComment(line);
		if (isSpace(line) || line.empty()) continue;

		if (line == "server:")
		{
			this->parseServer();
		}
		else
		{
			std::cerr << "[ERROR]: invalid keyword" << std::endl;
		}
	}
}

ConfigFile::ConfigFile(void)
{
	std::string line;
	const char *pathname = "./webserv.conf";

	file.open(pathname);
	if (!file.is_open())
	{
		std::cerr << "[ERROR]: " << strerror(errno) << ": " << pathname << std::endl;
		exit(1);
	}
	this->ss << file.rdbuf();
	while (!this->ss.eof())
	{
		line = getLine(this->ss);
		ConfigFile::cutComment(line);
		if (isSpace(line) || line.empty()) continue;

		if (line == "server:")
		{
			this->parseServer();
		}
		else
		{
			std::cerr << "[ERROR]: invalid keyword" << std::endl;
		}
	}
}

ConfigFile::~ConfigFile(void)
{
	this->file.close();
}

/* ------------------------- static methods ----------------- */
int			ConfigFile::toNumber(const std::string &str)
{
	std::stringstream ss(str);
	int	num;

	ss >> num;
	return num;
}
bool		ConfigFile::isNumber(const std::string &str)
{
	size_t count = std::count_if(str.begin(), str.end(), isdigit);
	return count == str.size();
}
bool		ConfigFile::isSpace(const std::string &str)
{
	size_t count = std::count_if(str.begin(), str.end(), isspace);
	return count == str.size();
}
Method		ConfigFile::isMethod(const std::string &method)
{
	const std::string methods[] = {"GET", "POST", "DELETE"};
	for (size_t i = 0; i < 3; i += 1)
	{
		if (methods[i] == method)
			return static_cast<Method>(i);
	}
	return static_cast<Method>(-1);
}

std::string	ConfigFile::getLine(std::stringstream &_ss)
{
	std::string line;
	std::getline(_ss, line);
	return line;
}
std::string	ConfigFile::getToken(std::stringstream &_ss)
{
	std::string token;
	_ss >> token;
	return token;
}
