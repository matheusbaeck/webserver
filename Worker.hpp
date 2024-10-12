
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
#include <sstream>


#include "Request.hpp"
#include "ALogger.hpp"

#define SERVER_PORT 5000
#define BUFFERSIZE 4096
#define MAX_EVENTS 10		//epoll_wait max events at time
#define BACKLOG 10			//listen param

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct epoll_event epoll_event;

class Worker : public ALogger
{
	private:
		int			m_serv_port;
		static int	m_instance_counter;
		int			m_id;
		sockaddr_in	m_addr;
		socklen_t	m_addrlen;
		int			m_serv_socket;

	public:
		Worker( void );
		Worker( const int port );
		Worker( const Worker & other );
		~Worker( void );

		/* Operators */
		Worker& 	operator=(const Worker& other);

		/* Acessors */
		sockaddr	*addr( void ) const;
		socklen_t	addrlen( void ) const;
		int			id( void ) const;
		int			sock( void ) const;
		int			port( void ) const;

		/* Methods */
		int			create_server_socket( void );
		int			setnonblocking( int );
		int			accept_connection( void );
		int			handle_read( int );


		class CreateSocketFunctor
		{
			public:
				void operator()(Worker& worker)
				{
					worker.create_server_socket();
				}
		};

		/* Logger */
		void				LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL);
		void				LogMessage(int logLevel, std::exception* ex = NULL);
		virtual std::string	GetType() const;


};

std::ostream	&operator<<( std::ostream &, const Worker & );

#endif
