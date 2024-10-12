#include "Request.hpp"

Request::Request(int fd) : m_fd(fd) {}

Request::Request(int fd, int port) : m_fd(fd), m_port(port) {}

Request::~Request() {}

int	Request::fd( void ) const {return (this->m_fd); }

int	Request::port( void ) const {return (this->m_port); }

void	Request::handler( void )
{
	char buffer[BUFFERSIZE];
	ssize_t bytes_read;

	bytes_read = read(this->m_fd, buffer, sizeof(buffer));
	if (bytes_read <= 0)
	{
		if (bytes_read == 0)
		{
			close(this->m_fd);
		}
		else
		{
			perror("read");
		}
		return ;
	}
	const char *response = "HTTP/1.1 200 OK\r\n"
							"Content-Type: text/plain\r\n"
							"Content-Length: 12\r\n"
							"\r\n"
							"Hello World!";
	write(this->m_fd, response, strlen(response));
	std:: cout << "request sent" << std::endl;

	//close(fd);
}

std::ostream	&operator<<( std::ostream &os, const Request &r )
{
	os << "request:" << r.port() << ":" << r.fd();
	return (os);
}