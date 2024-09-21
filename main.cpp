#include "Worker.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"
#include "Log.hpp"

int main( void )
{
	Log* logger = Log::getInstance("logfile.log");
	Selector &selector = Selector::getSelector();
	std::vector<std::vector<int> >	server_ports;
	std::string 					ports;
	ServerManager 					manager("{ {200, 800, 443}, {8080, 8443}, {3000, 3001} }");
	Selector::Functor 				RequestToQueue(&Selector::putEventsToQ);
	
	logger->logMessage(NULL, INFO, "Main starting ...");
	manager.LogMessage(INFO, "Main starting ..." );
	for ( ; ; ) {
		std::cout << "main loop" << std::endl;
		//manager.throwWorker(RequestToQueue);
		for (ServerIterator it = manager.getServers().begin() ; it != manager.getServers().end() ; ++it) {
			for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd() ; ++w ) {
				// std::cout << *w << "main line 20" <<std::endl;
				selector.putEventsToQ(*w, manager.getQueue());
			}
		}
		logger->logMessage(NULL, DEBUG, "Main starting ...");
		std::cout << "RequestHandler" << std::endl;
		manager.RequestHandler();
	}
	return(0);
}
