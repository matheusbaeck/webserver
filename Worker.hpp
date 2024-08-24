
#ifndef WORKER_HPP__
# define WORKER_HPP__

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
#include "Selector.hpp"

#define SERVER_PORT 2626
#define BUFFERSIZE 4096
#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT -1			//epoll_wait max time
#define BACKLOG 10			//listen param

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct epoll_event epoll_event;

class Selector;

class Worker
{
	private:
		Selector		&m_selector;
		const int		m_serv_port;
		sockaddr_in		m_addr;
		socklen_t		m_addrlen;
		int				m_serv_socket;

	public:
		/*after implementation of Server class, implement the following constructor*/
		// Worker( Selector &, Server &, int Port );
		Worker( Selector & );
		Worker( Selector &, int );
		~Worker( void );

		sockaddr	*addr( void ) const;
		socklen_t	addrlen( void ) const;
		int			sock( void ) const;
		int			port( void ) const;
		int			create_server_socket( void );
};

std::ostream	&operator<<( std::ostream &, const Worker & );

#endif
