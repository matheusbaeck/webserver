#include "Server.hpp"

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

Server::Server( std::vector<int> workers_port )
{
	LogMessage(DEBUG, "server up!");
	AddWorkerFunctor functor(this);
	std::for_each(workers_port.begin(), workers_port.end(), functor);
}

Server::~Server() {}

Server& Server::operator=(const Server& other)
{
	if (this != &other) // Check for self-assignment
	{
		// Call base class assignment operator if necessary
		ALogger::operator=(other);

		// Copy data members
		m_server_name = other.m_server_name;
		m_workers = other.m_workers;

		// Additional logging or handling if needed
		LogMessage(DEBUG, "Server copy assignment operator called");
	}
	return *this;
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


