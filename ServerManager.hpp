#ifndef SERVERMANAGER_HPP__
# define SERVERMANAGER_HPP__

#include <vector>

#include "Server.hpp"

class Server;

typedef std::vector<Server>::const_iterator ServerIterator;
typedef std::vector<Worker>::const_iterator WorkerIteratorInternal;

class ServerManager
{
	private:
		std::vector<Server> servers;
		std::queue<Request>	requests;

	public:
		// ServerManager( void );
		// ServerManager( int, ... );
		ServerManager( std::vector<std::vector<int> > );
		~ServerManager( void );

		void									addServer( const Server & );
		std::vector<Server> 					&getServers( void );
		std::vector<Request> 					&getQueue( void );
		void									RequestHandler( void );

		class AddServerFunctor {
			private:
				ServerManager* serverManager;
			public:
				AddServerFunctor(ServerManager* sm) : serverManager(sm) {}

				void operator()(const std::vector<int>& ports) {
					serverManager->addServer(Server(ports));
				}
		};

		class WorkerIterator
		{
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
};

#endif