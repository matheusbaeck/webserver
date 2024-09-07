#include "ServerManager.hpp"

ServerManager::ServerManager( void ) {}

ServerManager::ServerManager(int count, ...)
{
	va_list args;
	va_start(args, count);

	for (int i = 0; i < count; ++i) {
		Server server = va_arg(args, Server);
		this->addServer(server);
	}
	va_end(args);
}

ServerManager::ServerManager( std::vector<std::vector<int> > servers_ports )
{
	AddServerFunctor functor(this);
	std::for_each(servers_ports.begin(), servers_ports.end(), functor);
}

ServerManager::~ServerManager( void ) {}

void ServerManager::addServer(const Server& server) { servers.push_back(server); }

std::vector<Server>		&ServerManager::getServers() { return (this->servers); }
std::vector<Request>	&ServerManager::getQueue() { return (this->requests); }

void	ServerManager::RequestHanlder( void )
{
	while (this->requests.size() > 0)
	{
		this->requests.front().handler();
		this->requests.pop();
	}
}


/* ContainerIterator<std::vector<Worker>> ServerManager::allWorkersBegin()
{
	std::vector<Worker> allWorkers;
	for (std::vector<Server>::iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
		allWorkers.insert(allWorkers.end(), serverIt->getWorkers().begin(), serverIt->getWorkers().end());
	}
	return ContainerIterator<std::vector<Worker>>(allWorkers.begin());
}

ContainerIterator<std::vector<Worker>> ServerManager::allWorkersEnd()
{
	std::vector<Worker> allWorkers;
	for (std::vector<Server>::iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
		allWorkers.insert(allWorkers.end(), serverIt->getWorkers().begin(), serverIt->getWorkers().end());
	}
	return (ContainerIterator<std::vector<Worker>>(allWorkers.end()));
} */