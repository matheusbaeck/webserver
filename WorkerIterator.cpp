#include "ServerManager.hpp"

ServerManager::WorkerIterator::WorkerIterator(ServerIterator startServer, ServerIterator endServ) : currServ(startServer), endServ(endServ)
{
	if (currServ != endServ)
	{
		curr = currServ->workersBegin();
		advanceToNextValid();
	}
}

void ServerManager::WorkerIterator::advanceToNextValid()
{
	while (currServ != endServ && curr == currServ->workersEnd())
	{
		++currServ;
		if (currServ != endServ)
			curr = currServ->workersBegin();
	}
}

ServerIterator	ServerManager::WorkerIterator::curServer( void )
{
	return (this->currServ);
}

ServerIterator	ServerManager::WorkerIterator::endServer( void )
{
	return (this->endServ);
}

void	ServerManager::WorkerIterator::forEachWorker( void (*f)( const Worker & ) )
{
	for (; *this != WorkerIterator(endServ, endServ); ++(*this))
		f(**this);
}

void	ServerManager::WorkerIterator::forEachWorker( void (*f)( const Worker & , void *param ), void *param )
{
	for (; *this != WorkerIterator(endServ, endServ); ++(*this))
		f(**this, param);
}

bool ServerManager::WorkerIterator::operator!=(const WorkerIterator& other) const
{
	return (currServ != other.currServ || curr != other.curr);
}

const Worker& ServerManager::WorkerIterator::operator*( void ) const { return (*curr); }

ServerManager::WorkerIterator& ServerManager::WorkerIterator::operator++( void ) 
{
	++curr;
	advanceToNextValid();
	return (*this);
}