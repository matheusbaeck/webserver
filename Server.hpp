
#ifndef SERVER_HPP__
# define SERVER_HPP__

#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdarg>

#include "Worker.hpp"
//#include "WorkerInterator.hpp"

class Worker;

class WorkerInterator;

class Server
{
	private:
		std::string			m_server_name;
		std::vector<Worker>	m_workers;
		/* all other att of the server */

	public:
		Server( int, ... );
		Server( std::vector<int> );
		~Server( void );

		void							addWorker( const Worker & );
		std::vector<Worker>				&getWorkers( void );
		std::vector<Worker>::iterator	workersBegin( void );
		std::vector<Worker>::iterator	workersEnd( void );

		class AddWorkerFunctor {
			private:
				Server	*server;
			public:
				AddWorkerFunctor(Server *srv) : server(srv) {}

				void operator()(int port) {
					server->addWorker(Worker(port));
				}
		};
};


#endif
