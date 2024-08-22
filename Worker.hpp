
#ifndef WORKERS_HPP__
# define WORKERS_HPP_

#include <cstdio>
#include <cstdlib> // malloc
#include <cstring> // memset, bzero
#include <arpa/inet.h> // inet_pton [convert string IP to binary]
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
# include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <iomanip>

#define SERVER_PORT 2626
#define BUFFERSIZE 4096
#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT -1			//epoll_wait max time
#define BACKLOG 10			//listen param

typedef struct sockaddr_in serv;

class Worker
{
	private:
		serv	_addr;
		int		serv_socket;
	public:
		Worker(void);
		~Worker();

};

#endif