#include "Server.hpp"
#include "Selector.hpp"
#include "HttpRequest.hpp"

Server::Server(ConfigServer &configServer)
{
   for (size_t i = 0; i < configServer.getPorts().size(); i += 1)
   {
        this->_serv_ports.push_back(configServer.getPorts()[i]);
        std::cout << "Server listening on localhost: " << this->_serv_ports[i] << std::endl;
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
	if (this->_serv_sockets[pos] < 0)
    {
		std::cerr << "create_serv_sockets[pos]: " << strerror(errno) << std::endl;
		return -1;
	}
	int reuse = 1;
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
		std::cerr << "Couldn't bind socket " << this->_serv_sockets[pos] 
                    << " to port " << this->_serv_ports[pos] << ": " << strerror(errno) << std::endl;
		exit(1);
	}
	std::cout << "socket on fd " << this->_serv_sockets[pos] 
                << " bound to " << this->_serv_ports[pos] << ": " << strerror(errno)<< std::endl;
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

int Server::acceptClient(Selector& selector, int socketFD, int portFD)
{
    int client_fd = accept(socketFD, NULL, NULL);
    selector.getClientConfig()[client_fd] = this->getConfig();

    std::cout << "New client_fd " << client_fd << " accepted on port: " << portFD << std::endl;
    if (client_fd < 0)
    {
        std::cerr << "Failed to accept new connection: " << strerror(errno) << std::endl;
        return (-1);
    }
    // Adding the new client socket to epoll
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; 
    ev.data.fd = client_fd;
    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, client_fd, &ev) == -1) 
    {
        std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
        close(client_fd);
        return (-1);
    }
    return (0);
}

int Server::handleHTTPRequest(Selector& selector, int client_socket)
{
    char buffer[1024] = {0};
    int received_bytes = recv(client_socket, buffer, sizeof(buffer), MSG_DONTWAIT);
    std::cout << "bytes read: " << received_bytes  << " from fd " << client_socket << std::endl;
    if (received_bytes == 0)
    {
        epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, client_socket, NULL);
        selector.getClientConfig().erase(client_socket);
        close(client_socket);
        return (-1);
    }
    if (received_bytes < 0)
        return (-1);

    HttpRequest* incomingRequestHTTP = new HttpRequest();
    incomingRequestHTTP->setConfig(selector.getClientConfig()[client_socket]);
    std::cout << "Buffer is: " << buffer << std::endl;
    incomingRequestHTTP->setBuffer(buffer);
    std::string response = incomingRequestHTTP->handler();
    int sent_bytes = send(client_socket, response.c_str(), response.size(), 0);
    if (sent_bytes < 0) 
    {
        epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, client_socket, NULL);
        selector.getClientConfig().erase(client_socket);
        delete incomingRequestHTTP;
        close(client_socket);
        return (-1);
    }

    if (client_socket & (EPOLLERR| EPOLLHUP))
    {
        std::cerr << "Error on fd " <<  client_socket << ": " << strerror(errno) << std::endl;
        epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL,  client_socket, NULL);
        selector.getClientConfig().erase(client_socket);
        close(client_socket);
    }
    delete incomingRequestHTTP;
    return (0);
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


