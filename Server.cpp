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
    this->setnonblocking(this->_serv_sockets[pos]);
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

int Server::acceptConnection(Selector& selector, int socketFD, int portFD)
{
    int client_fd = accept(socketFD, NULL, NULL);

    std::cout << "New client_fd " << client_fd << " accepted on port: " << portFD << std::endl;
    if (client_fd < 0)
    {
        std::cerr << "Failed to accept new connection: " << strerror(errno) << std::endl;
        return (-1);
    }
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; 
    ev.data.fd = client_fd;
    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, client_fd, &ev) == -1) 
    {
        std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
        close(client_fd);
        return (-1);
    }
    selector.getActiveClients().insert(client_fd);
    selector.getClientConfig()[client_fd] = this->getConfig();
    return (0);
}

void Server::readClientRequest(Selector& selector, int clientFD)
{
    size_t                      pos = std::string::npos;
    static const std::string    RequestHeaderEnding = "\r\n\r\n";

    while (pos == std::string::npos) 
    {
        char buffer[1024];
        ssize_t count = read(clientFD, buffer, sizeof(buffer));
        std::cout << "bytes read from " << clientFD << ": " << count << std::endl;
        if (count == -1) 
            return; // not done reading everything yet, so return
        if (count == 0) 
        { 
            epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, clientFD, NULL);
            selector.getClientConfig().erase(clientFD);
            selector.getActiveClients().erase(clientFD);
            close(clientFD); 
            break; 
        } 
        selector.getRequests()[clientFD] += std::string(buffer, buffer + count);
        pos = selector.getRequests()[clientFD].find(RequestHeaderEnding);
        if (pos == std::string::npos) 
            continue;
        selector.setClientFdEvent(this, clientFD, WRITE);
    }
}



int Server::sendResponse(Selector& selector, int client_socket, std::string request)
{
    const char* buffer = request.c_str();
    if (request.size() == 0)
    {
        selector.getRequests()[client_socket].erase();
        return (-1);
    }
    std::cout << __func__ << " in " << __FILE__ << " | requestBuffer: " << buffer << std::endl;

    HttpRequest* incomingRequestHTTP = new HttpRequest();
    incomingRequestHTTP->setConfig(selector.getClientConfig()[client_socket]);
    incomingRequestHTTP->setBuffer(buffer);
    std::string response = incomingRequestHTTP->handler(selector, client_socket);
    int sent_bytes = send(client_socket, response.c_str(), response.size(), 0);
    if (sent_bytes < 0) 
    {
        this->removeClient(selector, client_socket);
        delete incomingRequestHTTP;
        return (-1);
    }
    delete incomingRequestHTTP;
    selector.getRequests().erase(client_socket);
    selector.setClientFdEvent(this, client_socket, READ);
    return (0);
}

std::string	toString(size_t num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

int Server::handleResponsePipe(Selector& selector, int pipeFd) 
{
    char buffer[4096];
    ssize_t bytesRead;
    cgiProcessInfo& cgiInfo = selector.getCgiProcessInfo(); // Cache CGI info
    int clientFd = cgiInfo._clientFd;

    // Read data from the pipe
    while ((bytesRead = read(pipeFd, buffer, sizeof(buffer))) > 0) 
    {
        ssize_t bytesSent = 0;
        while (bytesSent < bytesRead) 
        {
            ssize_t sent = send(clientFd, buffer + bytesSent, bytesRead - bytesSent, 0);
            if (sent == -1) 
                    break;
            else {
                // Fatal send error
                perror("send");
                close(cgiInfo._responsePipe);
                close(clientFd);
                selector.getRequests().erase(clientFd);
                return -1;
            }
            bytesSent += sent; // Update the amount of data sent
        }
    }
    if (bytesRead == 0) 
    {
        close(cgiInfo._responsePipe); // Close response pipe
        selector.setClientFdEvent(this, clientFd, READ);
    } 
    else
    {
        // Fatal read error
        perror("read");
        close(cgiInfo._responsePipe);
        close(clientFd);
        selector.getRequests().erase(clientFd);
        return -1;
    }

    return 1; // Successfully handled the pipe
}



/*int Server::handleResponsePipe(Selector& selector, int pipeFd)*/
/*{*/
/*    char buffer[4096];*/
/*    int clientFd = selector.getCgiProcessInfo().clientFd;*/
/*    ssize_t bytesRead;*/
/*    std::string response;*/
/**/
/*    while ((bytesRead = read(pipeFd, buffer, sizeof(buffer))) > 0) */
/*    {*/
/*        ssize_t bytesSent = 0;*/
/*        while (bytesSent < bytesRead)*/
/*        {*/
/*            ssize_t sent = send(clientFd, buffer + bytesSent, bytesRead - bytesSent, 0);*/
/*            if (sent == -1) // If send buffer is full, stop sending for now*/
/*                    break;*/
/*            else*/
/*            {*/
/*                // Handle send error*/
/*                    perror("send");*/
/*                    close(selector.getCgiProcessInfo().responsePipe.first);*/
/*                    close(clientFd);*/
/*                    selector.getRequests().erase(clientFd);*/
/*                    return -1;*/
/*            }*/
/*            bytesSent += sent; // Update the amount of data sent*/
/*        }*/
/*    }*/
/*    if (bytesRead == 0) */
/*    {*/
/*            close(selector.getCgiProcessInfo().responsePipe.first);*/
/*            selector.getRequests().erase(clientFd);*/
/*            if (fcntl(pipeFd, F_GETFD) == -1) {*/
/*                perror("Invalid clientFd");*/
/*            }*/
/*            selector.setClientFdEvent(this, pipeFd, READ);*/
/*    }*/
/*    else */
/*        removeClient(selector, clientFd);*/
/*    return 1;*/
/*}*/

void Server::removeClient(Selector& selector, int client_socket)
{
    epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, client_socket, NULL);
    selector.getClientConfig().erase(client_socket);
    selector.getActiveClients().erase(client_socket);
    selector.getRequests().erase(client_socket);
    close(client_socket);
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


