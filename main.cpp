#include "Worker.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"
#include "Log.hpp"
#include "Selector.hpp"
#include "ConfigFile.hpp"

#if 1
int main(int argc, char **argv)
{
	Log* logger = Log::getInstance("logfile.log");
	
	const char *pathname = ConfigFile::pathname;

	(void) pathname;
	if (argc > 2)
	{
		logger->logMessage(NULL, ERROR, "too many arguments");
		logger->logMessage(NULL, INFO, "Usage: ./webserv [config]");
		return 1;
	}
	if (argc == 2) pathname = argv[1];

	ConfigFile configFile(pathname);


	// TODO: each worker should has it's own server config.
	//

// 	std::vector<ConfigServer> configServers = configFile.getServers();

// 	for (size_t i = 0; i < configServers.size(); i += 1)
// 	{
// 		std::cout << configServers[i] << std::endl;
// 	}



	//return 0;


#if 1

	ServerManager	manager(configFile.getServers());

	Selector& selector = Selector::getSelector();
	
	manager.forEachWorker(Worker::CreateSocketFunctor());
	manager.forEachWorker(Selector::AddSocketFunctor());

	logger->logMessage(NULL, INFO, "WebServer started");


	for ( ; ; ) {
		manager.LogMessage(TRACE, "MainLoop" );
		//selector.processEvents(manager);
		for (std::vector<Server>::iterator it = manager.getServers().begin() ; it != manager.getServers().end() ; ++it)
		{
			selector.processEvents(*it);
		}
	}
#endif

	return(0);
}

#else

struct BodyRequest
{
	union {
		std::string *raw;
		std::map<std::string, std::string> *urlencoded;
	};

};

int main()
{
	std::string str;
	std::stringstream ss("ahmed");

	char c;
	ss.get(c);

	std::cout << c << std::endl;


	//std::generate(str.begin(), str.end(), ss.get);


	return 0;


	BodyRequest bodyRequest;
	

	bodyRequest.raw = new std::string("raw data");

	std::cout << "from string pointer: " << *bodyRequest.raw << std::endl;
	std::cout << "from map    pointer: " << *(std::string*)bodyRequest.urlencoded << std::endl;

	std::cout << "address of string: " << bodyRequest.raw << std::endl;
	std::cout << "address of map   : " << bodyRequest.raw << std::endl;

	delete bodyRequest.raw;


	return 0;
}
#endif
