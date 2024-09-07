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

		class WorkerIterator
		{
			private:
				ServerIterator			currServ;
				ServerIterator			endServ;
				WorkerIteratorInternal	curr;

				void advanceToNextValid( void );

			public:
				WorkerIterator(ServerIterator startServer, ServerIterator endServ);

				ServerIterator	curServer( void );
				ServerIterator	endServer( void );
				void	forEachWorker( void (*f)( const Worker & ) );
				void	forEachWorker( void (*f)( const Worker & , void *param ), void *param );

				bool 	operator!=( const WorkerIterator& other ) const;
				const 	Worker& operator*( void ) const;
				WorkerIterator&	operator++( void );
		};

	public:
		// ServerManager( void );
		// ServerManager( int, ... );
		ServerManager( std::vector<std::vector<int> > );
		~ServerManager( void );

		void					addServer( const Server & );
		std::vector<Server> 	&getServers( void );
		std::vector<Request> 	&getQueue( void );
		void					RequestHandler( void );
		void					forEachWorker(void (*f)( const Worker & )) const;
		void					forEachWorker(void (*f)( const Worker & worker, void* param ), void* param);
		void					throwWorker(void (*f)( const Worker & worker, std::queue<Request>& ));
		template <typename Func>
		void throwWorker(Func func) {
			for (ServerIterator it = servers.begin(); it != servers.end(); ++it) {
				for (WorkerIteratorInternal w = it->workersBegin(); w != it->workersEnd(); ++w) {
					func(*w, this->requests);
				}
			}
		}


		class AddServerFunctor {
			private:
				ServerManager* serverManager;
			public:
				AddServerFunctor(ServerManager* sm) : serverManager(sm) {}

				void operator()(const std::vector<int>& ports) {
					serverManager->addServer(Server(ports));
				}
		};
};

#endif