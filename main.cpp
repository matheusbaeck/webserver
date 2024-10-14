#include "Worker.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"
#include "Log.hpp"
#include "Selector.hpp"
#include "ConfigFile.hpp"

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


	std::vector<ConfigServer> configServers = configFile.getServers();

	for (size_t i = 0; i < configServers.size(); i += 1)
	{
		std::cout << "server[" << (i+1) << "]" << std::endl;
		std::cout << configServers[i] << std::endl;
	}


	return 0;
	ServerManager	manager(configFile.getPorts());

	manager.setConfig(&configFile);

	Selector& selector = Selector::getSelector();
	
	manager.forEachWorker(Worker::CreateSocketFunctor());
	manager.forEachWorker(Selector::AddSocketFunctor());

	logger->logMessage(NULL, INFO, "WebServer started");

	for ( ; ; ) {
		manager.LogMessage(TRACE, "MainLoop" );
		for (std::vector<Server>::iterator it = manager.getServers().begin() ; it != manager.getServers().end() ; ++it)
		{
			selector.processEvents(*it);
		}
	}

	return(0);
}
