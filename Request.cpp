#include "Request.hpp"

Request::Request(int fd) : m_fd(fd) {}

Request::Request(int fd, int port) : m_fd(fd), m_port(port) {}

Request::~Request() {}

int	Request::fd( void ) {return (this->m_fd); }

int	Request::port( void ) {return (this->m_port); }

void	Request::handler( void )
{

}

std::ostream	&operator<<( std::ostream &os, const Request &obj )
{
	os << "request:" << this->port() << ":" << this->fd();
	return (os);
}