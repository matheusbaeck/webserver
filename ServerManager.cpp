#include "ServerManager.hpp"

ServerManager::ServerManager( void ) {}

ServerManager::ServerManager( std::vector<std::vector<int>> servers_ports )
{
	std::for_each(servers_ports.begin(), servers_ports.end(), [this](const std::vector<int>& ports) {
		this->addServer(Server(ports));
	});
}

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

ServerManager::~ServerManager( void ) {}

void ServerManager::addServer(const Server& server) { servers.push_back(server); }