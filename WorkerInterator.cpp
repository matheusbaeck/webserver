#include "WorkerInterator.hpp"

WorkerIterator::WorkerIterator(ServerIterator startServer, ServerIterator endServ) : currServ(startServer), endServ(endServ)
{
	if (currServ != endServ)
	{
		curr = currServ->workersBegin();
		advanceToNextValid();
	}
}

void WorkerIterator::advanceToNextValid()
{
	while (currServ != endServ && curr == currServ->workersEnd())
	{
		++currServ;
		if (currServ != endServ)
			curr = currServ->workersBegin();
	}
}

void	WorkerIterator::forEachWorker( void (*f)( const Worker & ) )
{
	for (; *this != WorkerIterator(endServ, endServ); ++(*this))
		f(**this);
}

void	WorkerIterator::forEachWorker( void (*f)( const Worker & , void *param ), void *param )
{
	for (; *this != WorkerIterator(endServ, endServ); ++(*this))
		f(**this, param);
}

bool WorkerIterator::operator!=(const WorkerIterator& other) const
{
	return (currServ != other.currServ || curr != other.curr);
}

const Worker& WorkerIterator::operator*() const { return (*curr); }

WorkerIterator& WorkerIterator::operator++() 
{
	++curr;
	advanceToNextValid();
	return (*this);
}