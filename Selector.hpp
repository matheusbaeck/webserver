
#ifndef SELECTOR_HPP__
# define SELECTOR_HPP__

#include <cstdio>			//perror
#include <unistd.h>			//close
#include <fcntl.h>			//fcntl
#include <sys/epoll.h>		//epoll
#include <sys/types.h>		//accept
#include <sys/socket.h>		//aceppt

#include "Server.hpp"

#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT 200			//epoll_wait max time

typedef struct epoll_event epoll_event;



// this class is a singleton (ensures that a class has only one instance and provides a global point of access to that instance)
class Selector 
{
	private:
		static Selector	selector;
		epoll_event		_ev;
		epoll_event		_events[MAX_EVENTS];
		int				_nfds;
        int             _epollfd;

		Selector( void );

	public:
		~Selector( void );
		static Selector& getSelector( void ) { return selector; }
        
		/* Methods */
		void	addSocket(const Server *);
		void	processEvents (const std::vector<Server*> & servers);

       /* class epollInstanceFail : public std::exception{
            const char* what() const throw();
        };*/

};

#endif
