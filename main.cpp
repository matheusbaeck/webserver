#include "Worker.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"

void static printWorkerPort(const Worker & w)
{
	std::cout << w << std::endl;
}

int main( void )
{
	//Selector &selector = Selector::getSelector();
	std::vector<std::vector<int> >	server_ports;
	std::string 					ports;

	ServerManager manager("{ {200, 800, 443}, {8080, 8443}, {3000, 3001} }");
	manager.forEachWorker(printWorkerPort);

	Selector::Functor RequestToQueue(&Selector::putEventsToQ);
	manager.throwWorker(RequestToQueue);
	return(0);
}
