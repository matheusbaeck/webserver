#include "Worker.hpp"

int Worker::m_instance_counter = 0;

Worker::Worker( void ) : m_serv_port(SERVER_PORT), m_id(++m_instance_counter)
{
	oss() << *this << "Worker contructor";
	LogMessage(DEBUG);
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(SERVER_PORT);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
	this->m_serv_socket = create_server_socket(); /* handle error */
	Selector::getSelector().addSocket(this->m_serv_socket);
}

Worker::Worker( const int port ) : m_serv_port(port), m_id(++m_instance_counter)
{
	oss() << "Worker contructor";
	LogMessage(DEBUG);
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(port);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
	this->m_serv_socket = create_server_socket();
	Selector::getSelector().addSocket(this->m_serv_socket);
}

Worker::Worker( const Worker &other ) : m_id(++m_instance_counter)
{
	oss() << "Worker copy contructor " << other;
	LogMessage(DEBUG);
	*this = other;
}

Worker::~Worker()
{
	oss() << "Shutdown!";
	LogMessage(DEBUG);
	close(this->m_serv_socket);
}

Worker& Worker::operator=(const Worker& other)
{
	oss() << "Copy assign " << *this << " = " << other;
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
			oss() << "dup: " << other << " " << strerror(errno);
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
	return *this;
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

std::ostream	&operator<<( std::ostream &os, const Worker &w )
{
	os << "worker:" << w.id();
	return (os);
}