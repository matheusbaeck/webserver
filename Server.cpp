#include "Server.hpp"

int Server::m_instance_counter = 0;

Server::Server(int count, ...)
{
	va_list	args;

	va_start(args, count);
	for (int i = 0; i < count; ++i)
	{
		int port = va_arg(args, int);
		this->addWorker(Worker(port));
	}
	va_end(args);
}

// Server::Server( std::vector<uint16_t> workers_port ) : m_id(++m_instance_counter)
// {
// 	oss() << "Server constructor";
// 	LogMessage(DEBUG);
// 	AddWorkerFunctor functor(this);
// 	std::for_each(workers_port.begin(), workers_port.end(), functor);
// }

Server::Server( ConfigServer &configServer) : m_id(++m_instance_counter)
{
	oss() << "Server constructor";

	LogMessage(DEBUG);

	AddWorkerFunctor functor(this);

	std::for_each(configServer.getPorts().begin(), configServer.getPorts().end(), functor);
	m_configServer = configServer;
}

Server::Server( const Server &other ) : m_id(++m_instance_counter)
{
	oss() << "Copy constructor ";
	LogMessage(DEBUG);
	Server::operator=(other);
}

Server::~Server()
{ 
	oss() << "shutdown!";
	LogMessage(DEBUG);
}

Server& Server::operator=(const Server& other)
{
	oss() << "Copy assign "; //<< *this << " = " << other; >>> possible seg fault
	LogMessage(DEBUG);
	if (this != &other)
	{
		m_server_name = other.m_server_name;
		m_workers = other.m_workers;
		m_id      = other.m_id;
		m_configServer =  other.m_configServer;
	}
	return (*this);
}

int	Server::id( void ) const { return (this->m_id); }
std::vector<Worker>&	Server::getWorkers() { return (m_workers); }
std::vector<Worker>::iterator	Server::workersBegin() { return (m_workers.begin()); }
std::vector<Worker>::iterator	Server::workersEnd() { return (m_workers.end()); }
std::vector<Worker>::const_iterator	Server::workersBegin() const { return (m_workers.begin()); }
std::vector<Worker>::const_iterator	Server::workersEnd() const { return (m_workers.end()); }


void	Server::LogMessage(int logLevel, const std::string& message, std::exception* ex)
{
	logger->logMessage(this, logLevel, message, ex);
}

void	Server::LogMessage(int logLevel, std::exception* ex)
{
	logger->logMessage(this, logLevel, m_oss.str(), ex);
}

std::string	Server::GetType(void) const
{
	std::ostringstream oss;
	oss << "Server:" << m_id;
	return oss.str();
}

ConfigServer	&Server::getConfig(void)
{
	return m_configServer;
}

std::ostream &operator<<( std::ostream &os, const Server &obj )
{
	os << "server:" << obj.id();
	return (os);
}

