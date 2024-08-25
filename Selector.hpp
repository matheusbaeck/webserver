
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

#include "Request.hpp"
#include "Worker.hpp"

#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT -1			//epoll_wait max time

typedef struct epoll_event epoll_event;

class Request;

class Worker;

// this class is a singleton
class Selector
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
		void	putEventsToQ( const Worker &, std::queue<Request> & );
};

#endif
