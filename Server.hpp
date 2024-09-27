
#ifndef SERVER_HPP__
# define SERVER_HPP__

#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdarg>

#include "Worker.hpp"
#include "ALogger.hpp"

class Worker;

class Server : public ALogger
{
	private:
		std::string			m_server_name;
		std::vector<Worker>	m_workers;
		/* all other att of the server */

	public:
		Server( int, ... );
		Server( std::vector<int> );
		~Server( void );

		void								addWorker( const Worker & );
		std::vector<Worker>					&getWorkers( void );
		std::vector<Worker>::const_iterator	workersBegin( void ) const;
		std::vector<Worker>::const_iterator	workersEnd( void ) const;

		Server& operator=(const Server& other);

		class AddWorkerFunctor {
			private:
				Server	*server;
			public:
				AddWorkerFunctor(Server *srv) : server(srv) {}

				void operator()(int port) {
					server->addWorker(Worker(port));
				}
		};

		void LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL)
		{
			logger->logMessage(this, logLevel, message, ex);
		}

		virtual std::string GetType() const
		{
			return "Server:" + m_server_name;
		}
};

#endif
