#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <stdint.h>

#define PORT_MIN	0
#define PORT_MAX 	65535
#define ARR_LEN(arr) sizeof(arr) / sizeof(arr[0])

template<typename T>
void	print(const T &e)
{
	std::cout << "    " << e << std::endl;
}
template<typename T, typename FNC>
void	iter(const T &e, FNC fnc)
{
	for_each(e.begin(), e.end(), fnc);
}

template<typename T>
void	printKeyValue(const T &e)
{

	std::cout << "        key  : " << e.first  << std::endl;
	std::cout << "        value: " << e.second << std::endl;
}

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

class Route
{
	std::string path;
	std::string root;
	std::vector<Method> methods;
	std::vector<std::string> default_files;
	std::map<StatusCode, std::string> redirection;
	bool autoIndex;
public:
	Route(void);
	// Getters
	
	bool	getAutoIndex(void);
	const std::string &getPath(void) const;
	const std::string &getRoot(void) const;
	std::vector<Method> &getMethods(void);
	std::vector<std::string> &getDefaultFiles(void);
	std::map<StatusCode, std::string> &getRedirection(void);


	// Parsing
	void	parseMethods(const std::string &line);
	void	parseRedirection(const std::string &line);
	void	parseRoot(const std::string &line);
	void	parseAutoIndex(const std::string &line);
	void	parseDefaultFile(const std::string &line);

	void	setPath(const std::string &path);
};



class ConfigServer
{
	// General config
	size_t								clientBodySize;
	std::vector<uint16_t>				ports;
	std::vector<std::string>			serverNames;
	std::map<StatusCode, std::string>	error_pages;

	// Route config
	std::vector<Route> routes;

public:
	ConfigServer(void);


	// Getters
	size_t getClientBodySize();
	
	std::vector<Route>		 			&getRoutes(void);
	std::vector<uint16_t>				&getPorts(void);
	std::vector<std::string> 			&getServerNames(void);
	std::map<StatusCode, std::string>	&getErrorPages(void);


	// Parsing
	void		parseRoute(std::string path, std::stringstream &ss);
	void		parseClientBodySize(const std::string &line);
	void		parseErrorPages(const std::string &line);
	void		parsePorts(const std::string &line);
	void		parseServerNames(const std::string &line);
	std::string parseRoutePath(std::stringstream &ss);

};

class ConfigFile
{
	std::stringstream 	ss;
	std::ifstream 		file;
	std::vector<ConfigServer> servers;
public:
	ConfigFile(void);
	ConfigFile(const char *pathname);
	~ConfigFile(void);

	std::vector<ConfigServer>	&getServers(void);
	
	static	int			toNumber(const std::string &str);
	static	bool		isNumber(const std::string &str);
	static	bool		isSpace(const std::string &str);
	static	Method		isMethod(const std::string &method);
	static	std::string	getLine(std::stringstream &ss);
	static  std::string	getToken(std::stringstream &ss);
	static  std::string cutComment(std::string &line);

	void	printTree(void);
	void	parseServer(void);
};

#endif  // CONFIG_FILE_HPP
