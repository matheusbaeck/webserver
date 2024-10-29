
#ifndef SELECTOR_HPP__
# define SELECTOR_HPP__

#include <cstdio>			//perror
#include <unistd.h>			//close
#include <fcntl.h>			//fcntl
#include <sys/epoll.h>		//epoll
#include <sys/types.h>		//accept
#include <sys/socket.h>		//aceppt

#include "HttpRequest.hpp"
#include "Server.hpp"

#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT 200			//epoll_wait max time

typedef struct epoll_event epoll_event;

//class Request;


// this class is a singleton (ensures that a class has only one instance and provides a global point of access to that instance)
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
		static Selector& getSelector( void ) { return selector; }

		/* Methods */
		void	addSocket( const Server * );
		void	processEvents (std::vector<Server*> & servers);

		class	AddSocketFunctor
		{
			private:
				Selector* selector;
			public:
				AddSocketFunctor() : selector(&Selector::getSelector()) {}

		};


};

#endif
