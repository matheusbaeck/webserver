
#ifndef WORKERS_HPP__
# define WORKERS_HPP_

#include <cstdio>
#include <cstdlib> // malloc
#include <cstring> // memset, bzero
#include <arpa/inet.h> // inet_pton [convert string IP to binary]
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <queue>

#include "Request.hpp"

#define SERVER_PORT 2626
#define BUFFERSIZE 4096
#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT -1			//epoll_wait max time
#define BACKLOG 10			//listen param

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct epoll_event epoll_event;

class Worker
{
	private:
		sockaddr_in		m_addr;
		socklen_t		m_addrlen;
		int				m_serv_socket;
		int				m_serv_port;

		epoll_event     m_ev;
		epoll_event     m_events[MAX_EVENTS];
		int             m_nfds, m_epollfd;
	public:
		Worker( void );
		Worker( int );
		~Worker( void );

		sockaddr	*addr( void );
		int			create_server_socket( void );
		void		run( std::queue<Request> & );
};

#endif
