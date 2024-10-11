#include "Worker.hpp"

int Worker::m_instance_counter = 0;

Worker::Worker( void ) : m_serv_port(SERVER_PORT), m_id(++m_instance_counter)
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(SERVER_PORT);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
}

Worker::Worker( const int port ): m_serv_port(port), m_id(++m_instance_counter)
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(port);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
}

Worker::Worker( const Worker &other ) :
	m_serv_port(other.m_serv_port),
	m_id(++m_instance_counter),
	m_addr(other.m_addr),
	m_addrlen(other.m_addrlen)
{
	oss() << "Worker copy constructor "; // << other; >>> segfault
	LogMessage(DEBUG);
	
	m_serv_socket = dup(other.m_serv_socket);
	if (m_serv_socket < 0)
	{
		oss() << "dup: " << /*other <<  >>> seg fault */" " << strerror(errno);
		LogMessage(WARN);
		m_serv_socket = create_server_socket();
	}
	if (m_serv_socket < 0)
	{
		LogMessage(FATAL, "Couldn't open a server socket, we lost a worker");
	}
}

Worker::~Worker()
{
	oss() << "Shutdown!";
	LogMessage(DEBUG);
	close(this->m_serv_socket);
}

Worker& Worker::operator=(const Worker& other)
{
	oss() << "Copy assign "; // << *this << " = " << other; >>> seg fault
	LogMessage(DEBUG);
	if (this != &other)
	{
		this->m_serv_port = other.m_serv_port;
		this->m_addr = other.m_addr;
		this->m_addrlen = other.m_addrlen;

		close(this->m_serv_socket);
		this->m_serv_socket = dup(other.m_serv_socket);
		if (this->m_serv_socket < 0)
		{
			oss() << "dup: " << /*other <<    >>> seg fault*/ " " << strerror(errno);
			LogMessage(WARN);
			this->m_serv_socket = create_server_socket();
		}
		if (this->m_serv_socket < 0)
		{
			LogMessage(FATAL, "Could't open a server socket, we lost a worker");
		}
	}
	oss() << "Copy assign complete";
	LogMessage(DEBUG);
	return (*this);
}

sockaddr	*Worker::addr( void ) const { return ((sockaddr *)&this->m_addr); }

socklen_t	Worker::addrlen( void ) const { return (this->m_addrlen); }

int			Worker::id( void ) const { return (this->m_id); }

int			Worker::sock( void ) const { return (this->m_serv_socket); }

int			Worker::port( void ) const { return (this->m_serv_port); }

int Worker::create_server_socket( void )
{
	int addrlen = sizeof(*this->addr());
	this->m_serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	oss() << "create_serv_socket: open socket on fd " << this->m_serv_socket;
	LogMessage(DEBUG);
	if (this->m_serv_socket < 0)
	{
		oss() << "create_serv_socket: " << strerror(errno);
		LogMessage(ERROR);
	}
	int reuse = 1;
	if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		oss() << "create_serv_socket: " << strerror(errno);
		LogMessage(ERROR);
		return -1;
	}
	if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		oss() << "create_serv_socket: " << strerror(errno);
		LogMessage(ERROR);
		return -1;
	}
	// bzero(this->m_addr, addrlen);
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addr.sin_port = htons(m_serv_port);
	if (bind(this->m_serv_socket, this->addr(), addrlen) == -1)
	{
		oss() << "create_serv_socket: " << strerror(errno);
		LogMessage(ERROR);
	}
	if (listen(this->m_serv_socket, BACKLOG) < 0)
	{
		oss() << "create_serv_socket: " << strerror(errno);
		LogMessage(ERROR);
	}
	return (this->m_serv_socket);
}

int Worker::setnonblocking(int sockfd)
{
	int	flags;
	
	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	return (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK));
}

int Worker::accept_connection(void)
{
	int	client_socket;
	
	client_socket = accept(m_serv_socket, (struct sockaddr*)&m_addr, &m_addrlen);
	if (client_socket < 0)
	{
		oss() << "accept failed: " << strerror(errno);
		LogMessage(ERROR);
		return -1;
	}
	else
	{
		oss() << "Accepted connection on socket " << client_socket;
		LogMessage(DEBUG);
	}
	if (setnonblocking(client_socket) < 0)
	{
		oss() << "Failed to set client socket to non-blocking mode";
		LogMessage(ERROR);
		close(client_socket);
		return -1;
	}
	return (client_socket);
}

int Worker::handle_read(int client_socket)
{
	char	buffer[BUFFERSIZE];
	int		bytes_read;
	
	bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
	if (bytes_read < 0)
	{
		oss() << "read failed: " << strerror(errno);
		LogMessage(ERROR);
	} 
	else if (bytes_read == 0)
	{
		oss() << "Client disconnected on socket " << client_socket;
		LogMessage(DEBUG);
		close(client_socket);
	} 
	else
	{
		oss() << "Received " << bytes_read << " bytes: " << std::string(buffer, bytes_read);
		LogMessage(DEBUG);
	}
	return (bytes_read);
}

void	Worker::LogMessage(int logLevel, const std::string& message, std::exception* ex)
{
	logger->logMessage(this, logLevel, message, ex);
}

void	Worker::LogMessage(int logLevel, std::exception* ex)
{
	logger->logMessage(this, logLevel, m_oss.str(), ex);
}

std::string	Worker::GetType() const
{
	std::stringstream	ss;

	ss << "Worker:" << m_id;
	return (ss.str());
}

std::ostream	&operator<<( std::ostream &os, const Worker &w )
{
	os << "worker:" << w.id();
	return (os);
}