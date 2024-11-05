#include "Server.hpp"

//WATCH OUT WITH ports[0], we can have multiple ports
Server::Server(ConfigServer &configServer)
{
   for (size_t i = 0; i < configServer.getPorts().size(); i += 1)
   {
        this->_serv_ports.push_back(configServer.getPorts()[i]);
        std::cout << "Server listening on localhost:" << this->_serv_ports[i] << std::endl;
        this->_configServer = configServer;

        sockaddr_in element;
        element.sin_family = AF_INET;
        element.sin_port = htons(this->_serv_ports[i]);
        element.sin_addr.s_addr = htonl(INADDR_ANY);

        this->_addrs.push_back(element);
        this->_addrlens.push_back(sizeof(*this->getAddr(i)));
        this->create_server_socket(i);

   }
}

sockaddr* Server::getAddr(int pos) const 
{ 
    return ((sockaddr *)&this->_addrs[pos]); 
}

socklen_t   Server::getAddrlen(int pos) const 
    {
        return (this->_addrlens[pos]); 
}

std::vector<int> Server::getSockets(void) const 
{ 
    return (this->_serv_sockets); 
}

std::vector<uint16_t>	Server::getPorts(void) const 
{ 
    return (this->_serv_ports); 
}

int Server::create_server_socket(int pos)
{
	int addrlen = sizeof(*this->getAddr(pos));
	this->_serv_sockets.push_back(socket(AF_INET, SOCK_STREAM, 0));
	std::cout << "create_serv_sockets[pos]: open socket on fd " << this->_serv_sockets[pos] << std::endl;
	if (this->_serv_sockets[pos] < 0)
    {
		std::cerr << "create_serv_sockets[pos]: " << strerror(errno) << std::endl;
		return -1;
	}
	int reuse = 1;
	if (setsockopt(this->_serv_sockets[pos], SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) 
    {
		std::cerr << "create_serv_sockets[pos]: " << strerror(errno) << std::endl;
		return -1;
	}
	if (setsockopt(this->_serv_sockets[pos], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		std::cerr << "create_serv_sockets[pos]: " << strerror(errno) << std::endl;
		return -1;
	}
	// bzero(this->_addr, addrlen);
	this->_addrs[pos].sin_family = AF_INET;
	this->_addrs[pos].sin_addr.s_addr = htonl(INADDR_ANY);
	this->_addrs[pos].sin_port = htons(this->_serv_ports[pos]);
	if (bind(this->_serv_sockets[pos], this->getAddr(pos), addrlen) == -1)
	{
		std::cerr << "create_serv_sockets[pos]: " << strerror(errno) << std::endl;
		exit(1);
	}
	if (listen(this->_serv_sockets[pos], BACKLOG) < 0)
		std::cerr << "create_serv_sockets[pos]: " << strerror(errno) << std::endl;
	return (this->_serv_sockets[pos]);
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
	}return (bytes_read);
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


