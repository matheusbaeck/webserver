#include "Worker.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"

int main( void )
{
	Selector &selector = Selector::getSelector();
	std::vector<std::vector<int> >	server_ports;
	std::string 					ports;
	ServerManager 					manager("{ {200, 800, 443}, {8080, 8443}, {3000, 3001} }");
	Selector::Functor 				RequestToQueue(&Selector::putEventsToQ);
	
	for ( ; ; ) {
		std::cout << "main loop" << std::endl;
		//manager.throwWorker(RequestToQueue);
		for (ServerIterator it = manager.getServers().begin() ; it != manager.getServers().end() ; ++it) {
			for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd() ; ++w ) {
				std::cout << *w << std::endl;
				selector.putEventsToQ(*w, manager.getQueue());
			}
		}
		std::cout << "RequestHandler" << std::endl;
		manager.RequestHandler();
	}
	return(0);
}
