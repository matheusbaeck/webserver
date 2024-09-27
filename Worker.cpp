#include "Worker.hpp"

Worker::Worker( void ) : m_serv_port(SERVER_PORT)
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(SERVER_PORT);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
	this->m_serv_socket = create_server_socket(); /* handle error */
	Selector::getSelector().addSocket(this->m_serv_socket);
	LogMessage(DEBUG, "worker up!");
}

Worker::Worker( const int port ) : m_serv_port(port)
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(port);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
	this->m_serv_socket = create_server_socket(); /* handle error */
	Selector::getSelector().addSocket(this->m_serv_socket);
	LogMessage(DEBUG, "worker up!");

}

Worker::~Worker()
{
	close(this->m_serv_socket);
}

Worker& Worker::operator=(const Worker& other)
{
	if (this != &other)
	{
		this->m_addr = other.m_addr;
		this->m_addrlen = other.m_addrlen;

		close(this->m_serv_socket);
		this->m_serv_socket = create_server_socket();
		if (this->m_serv_socket < 0)
		{
			LogMessage(ERROR, "Failed to create server socket in assignment operator");
		}
	}
	return *this;
}

sockaddr	*Worker::addr( void ) const { return ((sockaddr *)&this->m_addr); }

socklen_t	Worker::addrlen( void ) const { return (this->m_addrlen); }

int			Worker::sock( void ) const { return (this->m_serv_socket); }

int			Worker::port( void ) const { return (this->m_serv_port); }

int Worker::create_server_socket( void )
{
	int addrlen = sizeof(*this->addr());
	this->m_serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	std::cout << "open on fd:" << m_serv_socket << std::endl;
	if (this->m_serv_socket < 0)
	{
		LogMessage(ERROR, "fail to open socket");
	}
	// int reuse = 1;
	// if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
	// 	perror("setsockopt");
	// 	return -1;
	// }
	//bzero(this->m_addr, addrlen);
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addr.sin_port = htons(m_serv_port);
	if (bind(this->m_serv_socket, this->addr(), addrlen) == -1)
	{
		LogMessage(ERROR, "fail to bind socket");
	}
	if (listen(this->m_serv_socket, BACKLOG) < 0)
	{
		LogMessage(ERROR, "fail on listening on socket");
	}
	return (this->m_serv_socket);
}

std::ostream	&operator<<( std::ostream &os, const Worker &w )
{
	os << "worker:" << w.port() << ":" << w.sock();
	return (os);
}