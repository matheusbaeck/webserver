#include "Worker.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Templates.hpp"

void static printWorkerPort(const Worker & w)
{
	std::cout << "worker: " << w.port() << std::endl;
}

int main( void )
{
	//Selector &selector = Selector::getSelector();
	std::vector<std::vector<int> >	server_ports;
	std::string 					ports;
	
	ports = "{ {80, 443}, {8080, 8443}, {3000, 3001} }";
	initializeContainer(server_ports, ports);

	ServerManager manager(server_ports);
	manager.forEachWorker(printWorkerPort);

	Selector::Functor functor(&Selector::putEventsToQ);
	manager.throwWorker(functor);
	return(0);
}
