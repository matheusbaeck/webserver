#include "Worker.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"
#include "Log.hpp"

int main( void )
{
	Log* logger = Log::getInstance("logfile.log");
	//Selector &selector = Selector::getSelector();
	std::vector<std::vector<int> >	server_ports;
	std::string 					ports;
	ServerManager 					manager("{ {200, 800, 443}, {8080, 8443}, {3000, 3001} }");
	Selector::Functor 				RequestToQueue(&Selector::putEventsToQ);
	
	logger->logMessage(NULL, INFO, "WebServer started");
	for ( ; ; ) {
		manager.LogMessage(TRACE, "MainLoop" );
		manager.throwWorker(RequestToQueue); /* foreach (ServerManager::std::vector<Server::Worker::iterator> w) => Sellector::RequestToQueue(w, ServerManager::queue) */
		// for (ServerIterator it = manager.getServers().begin() ; it != manager.getServers().end() ; ++it) {
		// 	for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd() ; ++w ) {
		// 		// std::cout << *w <<std::endl;
		// 		selector.putEventsToQ(*w, manager.getQueue());
		// 	}
		// }
		/* can add an bool on the manager to skip this if there is no new request ??*/
		manager.RequestHandler();
	}
	return(0);
}
