#include "Socket.hpp"

Socket::Socket() : _domain(AF_INET), _type(SOCK_STREAM), _protocol(0)
{
    this->_sockfd = socket(_domain, _type, _protocol);
    if (this->_sockfd < 0)
	{
		perror("socket");
		/* handle error */
	}

}

Socket::~Socket( void ) { close(this->_sockfd); }

int Socket::fd( void ) { return (this->_sockfd); }

void    Socket::setNonBlocking( void )
{
    int	flags;

	flags = fcntl(this->_sockfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL)");
		/* handle error*/
	}
}
