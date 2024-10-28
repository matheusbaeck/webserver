
#ifndef SELECTOR_HPP__
# define SELECTOR_HPP__

#include <cstdio>			//perror
#include <unistd.h>			//close
#include <fcntl.h>			//fcntl
#include <sys/epoll.h>		//epoll
#include <sys/types.h>		//accept
#include <sys/socket.h>		//aceppt
#include <queue>			//queue
#include <iostream>			//std::cout

#include "ALogger.hpp"
#include "HttpRequest.hpp"
#include "Worker.hpp"
#include "Server.hpp"
#include <map>
#include "ServerManager.hpp"

#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT 200			//epoll_wait max time

typedef struct epoll_event epoll_event;

//class Request;


// this class is a singleton (ensures that a class has only one instance and provides a global point of access to that instance)
class Selector : public ALogger
{
	private:
		static Selector	selector;
		epoll_event		m_ev;
		epoll_event		m_events[MAX_EVENTS];
		int				m_nfds, m_epollfd;

		std::map<int, Worker*>	m_fd_to_worker_map;

		Selector( void );

	public:
		~Selector( void );
		static Selector& getSelector( void ) { return selector; }

		/* Methods */
		void	addSocket( const Worker & );
		void	processEvents( Server & );
		void	processEvents( ServerManager &);
		Worker*	getWorkerByFd( int ) const;

		class	AddSocketFunctor
		{
			private:
				Selector* selector;
			public:
				AddSocketFunctor() : selector(&Selector::getSelector()) {}

				void operator()(const Worker &worker) const
				{
					selector->addSocket(worker);
				}
		};


		/* Logger */
		void	LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL);
		void	LogMessage(int logLevel, std::exception* ex = NULL);
		virtual	std::string GetType() const;
};

#endif
