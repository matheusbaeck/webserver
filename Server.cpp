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
	AddWorkerFunctor functor(this);
	std::for_each(workers_port.begin(), workers_port.end(), functor);
}

Server::~Server() {}

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


