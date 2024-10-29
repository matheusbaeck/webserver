#include "Server.hpp"

Server::Server( ConfigServer &configServer )
{
    std::vector<uint16_t> ports = configServer.getPorts();
    //WATCH OUT WITH ports[0], we can have multiple ports
	std::cout << "Server listening on localhost:" << ports[0] << std::endl;

    this->m_serv_port = ports[0];
    this->m_configServer = configServer;
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(ports[0]);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
    this->create_server_socket();
}

sockaddr* Server::addr( void ) const 
{ 
    return ((sockaddr *)&this->m_addr); 
}

socklen_t   Server::addrlen( void ) const 
{
    return (this->m_addrlen); 
}

int Server::id( void ) const 
{ 
    return (this->m_id); 
}

int Server::sock( void ) const 
{ 
    return (this->m_serv_socket); 
}

int	Server::port( void ) const 
{ 
    return (this->m_serv_port); 
}

int Server::create_server_socket( void )
{
	int addrlen = sizeof(*this->addr());
	this->m_serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	std::cout << "create_serv_socket: open socket on fd " << this->m_serv_socket << std::endl;
	if (this->m_serv_socket < 0)
    {
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		return -1;
	}
	int reuse = 1;
	if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) 
    {
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		return -1;
	}
	if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		return -1;
	}
	// bzero(this->m_addr, addrlen);
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addr.sin_port = htons(m_serv_port);
	if (bind(this->m_serv_socket, this->addr(), addrlen) == -1)
	{
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		exit(1);
	}
	if (listen(this->m_serv_socket, BACKLOG) < 0)
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
	return (this->m_serv_socket);
}

int Server::setnonblocking(int sockfd)
{
	int	flags;
	
	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	return (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK));
}

int Server::handle_read(int client_socket)
{
	char	buffer[BUFFERSIZE];
	int		bytes_read;
	
	bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
	if (bytes_read < 0)
	{
		std::cerr << "read failed: " << strerror(errno) << std::endl;
	} 
	else if (bytes_read == 0)
	{
        std::cerr << "Client disconnected on socket " << client_socket << std::endl;
		close(client_socket);
	} 
	else
	{
		std::cout << "Received " << bytes_read << " bytes: " << std::string(buffer, bytes_read) << std::endl;
	}
	return (bytes_read);
}

Server::Server( const Server &other )
{
	Server::operator=(other);
}


Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		m_server_name = other.m_server_name;
		m_id      = other.m_id;
		m_configServer =  other.m_configServer;
	}
	return (*this);
}


ConfigServer&   Server::getConfig(void)
{
	return m_configServer;
}

Server::~Server() {}

std::ostream &operator<<( std::ostream &os, const Server &obj )
{
	os << "server:" << obj.id();
	return (os);
}

