#include "Worker.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"
#include "Log.hpp"
#include "Selector.hpp"

int main( void )
{
	Log* logger = Log::getInstance("logfile.log");
	Selector& selector = Selector::getSelector();
	std::vector<std::vector<int> >	server_ports;
	ServerManager 					manager("{ {2000, 8000, 4430}, {8080, 8443}, {3000, 3001} }");
	
	manager.forEachWorker(Worker::CreateSocketFunctor());
	manager.forEachWorker(Selector::AddSocketFunctor());

	logger->logMessage(NULL, INFO, "WebServer started");
	for ( ; ; ) {
		manager.LogMessage(TRACE, "MainLoop" );
		selector.processEvents(manager.getQueue());
		manager.RequestHandler();
	}
	
	return(0);
}

// int main(void)
// {
//     Log* logger = Log::getInstance("logfile.log");
//     Selector& selector = Selector::getSelector();
    
//     // Create a single worker for port 8080
//     Worker worker(8080);
//     worker.create_server_socket();
//     selector.addSocket(worker);

//     logger->logMessage(NULL, INFO, "WebServer started on port 8080");
    
//     std::queue<Request> requestQueue;
//     for (;;) {
//         selector.processEvents(requestQueue);
//         // Process any requests in the queue if needed
//     }
    
//     return 0;
// }
