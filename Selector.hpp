
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

#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT 200			//epoll_wait max time

typedef struct epoll_event epoll_event;

//class Request;

class Worker;

// this class is a singleton (ensures that a class has only one instance and provides a global point of access to that instance)
class Selector : public ALogger
{
	private:
		static Selector	selector;
		epoll_event		m_ev;
		epoll_event		m_events[MAX_EVENTS];
		int				m_nfds, m_epollfd;
		
		Selector( void );
	public:
		~Selector( void );

		static Selector& getSelector() { return selector; }

		void	addSocket( int ); //This adds the listening socket to the list of file descriptors being monitored by the epoll instance.
		void	putEventsToQ( const Worker &, std::queue<HttpRequest> & );
		void	setnonblocking(int conn_sock);

		class Functor
		{
			private:
				Selector*	selector;
				void (Selector::*putEventsToQ)(const Worker &, std::queue<HttpRequest> &);
			public:
				Functor(void (Selector::*f)(const Worker &, std::queue<HttpRequest> &))
					: selector(&Selector::getSelector()), putEventsToQ(f) {}

				//void operator()(const Worker &worker, std::queue<Request> &queue) const
				void operator()(const Worker &worker, std::queue<HttpRequest> &queue) const
				{
					
					(selector->*putEventsToQ)(worker, queue);
				}
		};

		void LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL)
		{
			logger->logMessage(this, logLevel, message, ex);
		}

		void LogMessage(int logLevel, std::exception* ex = NULL)
		{
			logger->logMessage(this, logLevel, m_oss.str(), ex);
		}

		virtual std::string GetType() const
		{
			return "Selector";
		}

};

#endif
