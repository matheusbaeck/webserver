#ifndef WORKERINTERATOR_HPP__
# define WORKERINTERATOR_HPP__

#include <vector>
#include "Worker.hpp"
#include "Server.hpp"



class WorkerIterator
{
	typedef std::vector<Server>::const_iterator ServerIterator;
	typedef std::vector<Worker>::const_iterator WorkerIteratorInternal;

	private:
		ServerIterator			currServ;
		ServerIterator			endServ;
		WorkerIteratorInternal	curr;

		void advanceToNextValid( void );

	public:
		WorkerIterator(ServerIterator startServer, ServerIterator endServ);

		void	forEachWorker( void (*f)( const Worker & ) );
		void	forEachWorker( void (*f)( const Worker & , void *param ), void *param );

		bool 	operator!=( const WorkerIterator& other ) const;
		const 	Worker& operator*( void ) const;
		WorkerIterator&	operator++( void );
};

#endif