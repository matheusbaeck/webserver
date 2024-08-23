#ifndef SOCKET_HPP__
# define SOCKET_HPP__

#include <sys/types.h>		//socket
#include <sys/socket.h>		//socket
#include <stdio.h>			//perror
#include <unistd.h>			//close, fnctl
#include <fcntl.h>			//fnctl

class Socket
{
	private:
		int	_sockfd;
		int	_domain;
		int	_type;
		int	_protocol;
	public:
		Socket( void );
		~Socket( void );

		int		fd( void );
		void	setNonBlocking( void );
};

#endif