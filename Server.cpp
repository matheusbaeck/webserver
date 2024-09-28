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

Server::Server( std::vector<int> workers_port ) : m_id(++m_instance_counter)
{
	oss() << "Server constructor";
	LogMessage(DEBUG);
	AddWorkerFunctor functor(this);
	std::for_each(workers_port.begin(), workers_port.end(), functor);
}

Server::Server( const Server &other ) : m_id(++m_instance_counter)
{
	oss() << "Copy constructor " << other;
	LogMessage(DEBUG);
	*this = other;
}

Server::~Server()
{ 
	oss() << "shutdown!";
	LogMessage(DEBUG);
}

int Server::id( void ) const
{
	return (this->m_id);
}

Server& Server::operator=(const Server& other)
{
	oss() << "Copy assign " << *this << " = " << other;
	LogMessage(DEBUG);
	if (this != &other)
	{
		m_server_name = other.m_server_name;
		m_workers = other.m_workers;
	}
	return (*this);
}

void Server::addWorker(const Worker& worker)
{
	this->m_workers.push_back(worker);
}

std::vector<Worker> &Server::getWorkers()
{
	return (m_workers);
}

std::vector<Worker>::const_iterator Server::workersBegin() const
{
	return (m_workers.begin());
}
std::vector<Worker>::const_iterator Server::workersEnd() const
{
	return (m_workers.end());
}

std::ostream &operator<<( std::ostream &os, const Server &obj )
{
	os << "server:" << obj.id();
	return (os);
}

