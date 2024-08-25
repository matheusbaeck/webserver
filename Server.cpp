#include "Server.hpp"

Server::Server( void ) {}

Server::Server(std::vector<int> workers_port)
{
	std::for_each(workers_port.begin(), workers_port.end(), [this](int port) {
		this->addWorker(Worker(port));
	});
}

Server::Server( int count, ... )
{
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; ++i) {
		int port = va_arg(args, int);
		this->addWorker(Worker(port));
	}
	va_end(args);
}

Server::~Server() {}

void Server::addWorker(const Worker& worker)
{
	this->m_workers.push_back(worker);
}