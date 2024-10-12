#include "Worker.hpp"
//#include "Request.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"
#include "Log.hpp"

int main( void )
{

#if 1
	Log* logger = Log::getInstance("logfile.log");

	(void) logger;


	ServerManager 					manager("{{2626}}");

	Selector::Functor 				RequestToQueue(&Selector::putEventsToQ);

	
	logger->logMessage(NULL, INFO, "WebServer started");

	std::cout << "size of servers: " << manager.getServers().size() << std::endl;


	Server *server = &manager.getServers()[0];
	Worker *worker = &server->getWorkers()[0];

	std::queue<int> clients;

	while (1)
	{
		std::cout << "size of clients: " << clients.size() << std::endl;
		if (worker)
		{
			int	clientfd = accept(worker->sock(), NULL, NULL);
			clients.push(clientfd);
		}
		if (clients.size() == 2)
			break;
	}
	while (!clients.empty())
	{
		send(clients.front(), "PONG", 4, 0);
		clients.pop();
	}

		

	

#if 0
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
#endif
#endif
	return(0);
}
