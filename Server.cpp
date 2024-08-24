#include "Server.hpp"

Server::Server( void ) {}

Server::~Server() {}

void Server::addWorker(const Worker& worker)
{
	this->m_workers.push_back(worker);
}