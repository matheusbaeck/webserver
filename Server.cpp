#include "Server.hpp"

//WATCH OUT WITH ports[0], we can have multiple ports
Server::Server(ConfigServer &configServer)
{
   // int nb_ports = configServer.getPorts().size();
   std::vector<uint16_t> ports = configServer.getPorts();
    std::cout << "Server listening on localhost:" << ports[0] << std::endl;

    this->_serv_port = ports[0]; 
    this->_configServer = configServer;
    this->_addr.sin_family = AF_INET;
    this->_addr.sin_port = htons(ports[0]);
    this->_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->_addrlen = sizeof(*this->getAddr());
    this->create_server_socket();
}

sockaddr* Server::getAddr(void) const 
{ 
    return ((sockaddr *)&this->_addr); 
}

socklen_t   Server::getAddrlen(void) const 
{
    return (this->_addrlen); 
}

int Server::getSock(void) const 
{ 
    return (this->_serv_socket); 
}

int	Server::getPorts(void) const 
{ 
    return (this->_serv_port); 
}

int Server::create_server_socket(void)
{
	int addrlen = sizeof(*this->getAddr());
	this->_serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	std::cout << "create_serv_socket: open socket on fd " << this->_serv_socket << std::endl;
	if (this->_serv_socket < 0)
    {
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		return -1;
	}
	int reuse = 1;
	if (setsockopt(this->_serv_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) 
    {
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		return -1;
	}
	if (setsockopt(this->_serv_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		return -1;
	}
	// bzero(this->_addr, addrlen);
	this->_addr.sin_family = AF_INET;
	this->_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->_addr.sin_port = htons(_serv_port);
	if (bind(this->_serv_socket, this->getAddr(), addrlen) == -1)
	{
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
		exit(1);
	}
	if (listen(this->_serv_socket, BACKLOG) < 0)
		std::cerr << "create_serv_socket: " << strerror(errno) << std::endl;
	return (this->_serv_socket);
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
		_server_name = other._server_name;
		_configServer =  other._configServer;
	}
	return (*this);
}


ConfigServer&   Server::getConfig(void)
{
	return _configServer;
}

Server::~Server() {}


