#include "ServerManager.hpp"

// ServerManager::ServerManager( void ) {}

// ServerManager::ServerManager(int count, ...)
// {
// 	va_list args;
// 	va_start(args, count);

// 	for (int i = 0; i < count; ++i) {
// 		Server server = va_arg(args, Server);
// 		this->addServer(server);
// 	}
// 	va_end(args);
// }

ServerManager::ServerManager( std::string ports )
{
	std::vector<std::vector<int> > server_ports;
	AddServerFunctor functor(this);

	server_ports = createVector( ports );
	std::for_each(server_ports.begin(), server_ports.end(), functor);
}

ServerManager::ServerManager( std::vector<std::vector<int> > server_ports )
{
	AddServerFunctor functor(this);
	std::for_each(server_ports.begin(), server_ports.end(), functor);
}

ServerManager::~ServerManager( void ) {}

void ServerManager::addServer(const Server& server) { servers.push_back(server); }

std::vector<Server>		&ServerManager::getServers() { return (this->servers); }
//std::vector<Request>	&ServerManager::getQueue() { return (this->requests); }

void	ServerManager::RequestHandler( void )
{
	while (this->requests.size() > 0)
	{
		this->requests.front().handler();
		this->requests.pop();
	}
}

/* void ServerManager::forEachWorker(void (*f)( const Worker & worker )) const
{
	for (ServerManager::WorkerIterator it(servers.begin(), servers.end()) ; it.curServer() != it.endServer() ; ++it) {
		f(*it);
	}
} */

void ServerManager::forEachWorker(void (*f)( const Worker & worker )) const
{
	for (ServerIterator it = this->servers.begin() ; it != servers.end() ; ++it) {
		for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd() ; ++w ) {
			f(*w);
		}
	}
}

void ServerManager::forEachWorker(void (*f)( const Worker & worker, void* param ), void* param)
{
	for (ServerIterator it = this->servers.begin() ; it != servers.end() ; ++it) {
		for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd() ; ++w ) {
			f(*w, param);
		}
	}
}

void	ServerManager::throwWorker(void (*f)( const Worker & worker, std::queue<Request>& ))
{
	for (ServerIterator it = this->servers.begin() ; it != servers.end() ; ++it) {
		for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd() ; ++w ) {
			f(*w, this->requests);
		}
	}
}

std::vector<std::vector<int> > createVector(const std::string& data)
{
	std::vector<std::vector<int> > container;
	std::istringstream stream(data);
	std::string line;

	std::getline(stream, line, '{');
	while (std::getline(stream, line, '{'))
	{
		std::vector<int> innerContainer;
		std::string element;

		while (std::getline(stream, element, ','))
		{
			size_t endPos = element.find('}');
			if (endPos != std::string::npos)
			{
				element = element.substr(0, endPos);
				innerContainer.push_back(atoi(element.c_str()));
				break;
			}
			else
				innerContainer.push_back(atoi(element.c_str()));
		}

		if (!innerContainer.empty())
			container.push_back(innerContainer);
	}
	return container;
}
