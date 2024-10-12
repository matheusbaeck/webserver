#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include "Tokenizer.hpp"

#define PEEK(e) std::cout << "peek -> " << (char)e << std::endl

// TODO: make route inherite from server config.

enum StatusCode
{
	OK 		   = 200,
	BREQUEST   = 400,
	FORBIDDEN  = 403,
	NFOUND     = 404,
	NALLOWED   = 405,
	CTOOLARGE  = 413,
	NSUPPORTED = 505,
};

enum Method {
	GET = 0,
	POST,
	DELETE,
};

class Route;


class ConfigServer
{
	// std::vector<size_t>	ports;

	std::vector<Route> routes;
	Tokenizer *tokenizer;
protected:

	bool	isResized;
	size_t			port;
	size_t			client_max_body_size;
	std::string root;
	std::vector<std::string> 		 	 server_names;
	std::vector<std::string>		 	 index;
	std::map<StatusCode, std::string>	 error_pages;
	std::map<StatusCode, std::string> 	 redirection;

public:
	/* ------- Constructors ------- */
	ConfigServer(void);
	~ConfigServer(void);
	ConfigServer(ConfigServer const &other);
	ConfigServer &operator=(ConfigServer const &other);

	/* ------- Methods ------- */
	Route	*getRoute(std::string const &path);
	std::vector<Route>	&getRoutes(void);

	void	parseListen(void);

	void	parseServerName(std::vector<std::string> &vec);
	void	parseIndex(void);

	void	parseRoot(void);

	void	parseErrorPage(void);

	void	parseBodySize(void);

	void	parseRoute(void);

	void	parse(void);


	/* ------- Getters ------- */
	size_t	getPort(void);
	size_t	getClientMaxBodySize(void);
	std::string	&getRoot(void);
	std::vector<std::string> &getServerNames(void);
	std::vector<std::string> &getIndex(void);
	std::map<StatusCode, std::string> &getErrorPages(void);


};

class Route : public ConfigServer
{
	//bool isResized;
	std::vector<Method>  methods;
	bool				 autoindex;

	Tokenizer *tokenizer;
public:
	// temp
	std::string path;

	Route(void);
	~Route(void);
	Route(Route const &other);
	Route(ConfigServer const &other);
	Route	&operator=(Route const &other);

	void	parseMethods(void);
	void	parseRedirection(void);
	void	parseAutoIndex(void);
	//void	parseIndex(void);

	bool				getAutoIndex(void);
	std::string 					&getRoot(void);
	std::vector<Method>				  &getMethods(void);
	std::map<StatusCode, std::string> &getRedirection(void);
	//std::vector<std::string>		  &getIndices(void);
};

class ConfigFile
{
  static Tokenizer tokenizer;
	std::ifstream file;
	std::vector<ConfigServer> servers;
public:

	static const char *delim;
	static const char *pathname;

	/* ------- Constructors ------- */
	ConfigFile(void);
	~ConfigFile(void);
	ConfigFile(const char *pathname);
	ConfigFile(ConfigFile const &other);
	ConfigFile	&operator=(ConfigFile const &other);


	/* ------- Methods ------- */
	void	parse(void);
	std::vector<ConfigServer>	&getServers(void);


	/* ------- Static Methods ------- */
	static bool 	 isNumber(std::string const &str);
	static size_t	 toNumber(std::string const &str);
	static Method 	 isMethod(std::string const &method);
	static Tokenizer &getTokenizer(void);
	
};

std::ostream	&operator<<(std::ostream &os, ConfigFile &obj);


#endif  // CONFIG_FILE_HPP
