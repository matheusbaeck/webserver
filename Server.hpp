
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
		static int			m_instance_counter;
		int					m_id;
		std::string			m_server_name;
		std::vector<Worker>	m_workers;
		/* all other att of the server */

	public:
		Server( int, ... );
		Server( std::vector<uint16_t> );
		Server( const Server & );
		~Server( void );

		/* Operators */
		Server& operator=(const Server& );

		/* Acessors */
		int									id( void ) const;
		std::vector<Worker>					&getWorkers( void );
		std::vector<Worker>::iterator		workersBegin( void );
		std::vector<Worker>::iterator		workersEnd( void );
		std::vector<Worker>::const_iterator	workersBegin( void ) const;
		std::vector<Worker>::const_iterator	workersEnd( void ) const;

		/* Methods */
		void addWorker( const Worker &w )
		{
			Worker temp;
			temp = w;
			oss() << "About to push_back. Vector size: " << m_workers.size();
			LogMessage(DEBUG);
			try 
			{
				m_workers.push_back(temp);
				oss() << "push_back successful. New size: " << m_workers.size();
				LogMessage(DEBUG);
			}
			catch (const std::exception& e)
			{
				oss() << "Exception during push_back: " << e.what();
				LogMessage(ERROR);
			}
			catch (...)
			{
				oss() << "Unknown exception during push_back";
				LogMessage(ERROR);
			}
		}

		class AddWorkerFunctor {
			private:
				Server	*server;
			public:
				AddWorkerFunctor(Server *srv) : server(srv) {}

				void operator()(int port) {
					Worker w(port);
					server->addWorker(w);
				}
		};

		void LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL);
		void LogMessage(int logLevel, std::exception* ex = NULL);
		virtual std::string GetType() const;
};

std::ostream &operator<<( std::ostream &, const Server & );

#endif