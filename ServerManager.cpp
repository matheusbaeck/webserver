#include "ServerManager.hpp"

ServerManager::ServerManager( void ) {}

ServerManager::ServerManager( const std::string ports )
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
std::queue<Request>	&ServerManager::getQueue(void) { return this->requests; }

void	ServerManager::RequestHandler( void )
{
	while (this->requests.size() > 0)
	{
		this->requests.front().handler();
		this->requests.pop();
	}
}

void ServerManager::addServer(const Server& server)
{ 
	servers.push_back(server);
}

std::vector<std::vector<int> >	ServerManager::createVector(const std::string& data)
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
