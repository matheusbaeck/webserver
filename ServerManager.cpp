#include "ServerManager.hpp"

ConfigFile *ServerManager::configFile;

ServerManager::ServerManager( void ) {}

ServerManager::ServerManager( std::vector<std::vector<uint16_t> > server_ports )
{
	AddServerFunctor functor(this);
	std::for_each(server_ports.begin(), server_ports.end(), functor);
}

ServerManager::~ServerManager( void ) {}

ServerManager& ServerManager::operator=(const ServerManager& other)
{
	if (this != &other)
	{
		this->servers = other.servers;
		this->requests = other.requests;
		LogMessage(DEBUG, "ServerManager copy assignment operator called");
	}
	return *this;
}


std::vector<Server>	&ServerManager::getServers() { return (this->servers); }
std::queue<HttpRequest>	&ServerManager::getQueue(void) { return this->requests; }

void ServerManager::addServer(const Server& server)
{ 
	servers.push_back(server);
}

void	ServerManager::LogMessage(int logLevel, const std::string& message, std::exception* ex)
{
	logger->logMessage(this, logLevel, message, ex);
}

void	ServerManager::LogMessage(int logLevel, std::exception* ex)
{
	logger->logMessage(this, logLevel, m_oss.str(), ex);
}

std::string	ServerManager::GetType( void ) const
{
	return "ServerManager";
}
