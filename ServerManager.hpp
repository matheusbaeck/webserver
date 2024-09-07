#ifndef SERVERMANAGER_HPP__
# define SERVERMANAGER_HPP__

#include <vector>

#include "Server.hpp"
#include "WorkerInterator.hpp"


class Server;

class ServerManager : private WorkerInterator
{
	private:
		std::vector<Server> servers;
		std::queue<Request>	requests;

	public:
		ServerManager( void );
		ServerManager( int, ... );
		ServerManager( std::vector<std::vector<int> > );
		~ServerManager( void );

		void									addServer( const Server & );
		std::vector<Server> 					&getServers( void );
		std::vector<Request> 					&getQueue( void );
		void									RequestHanlder( void );
		// ContainerIterator<std::vector<Worker>>	allWorkersBegin( void );
		// ContainerIterator<std::vector<Worker>>	allWorkersEnd( void );

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