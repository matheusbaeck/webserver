#include "CgiHandler.hpp"
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
    
        if (this->create_server_socket(i) == -1)
            throw std::runtime_error("Error when creating socket");
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
		return (-1);
	}
	std::cout << "socket on fd " << this->_serv_sockets[pos] 
                << " bound to " << this->_serv_ports[pos] << ": " << strerror(errno)<< std::endl;
	if (listen(this->_serv_sockets[pos], SOMAXCONN) < 0)
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
    while (true) 
    {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(socketFD, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientFd == -1) 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
                break;
            else 
            {
                perror("accept");
                return -1;
            }
        }
        if (clientFd < 0)
        {
            std::cerr << "Failed to accept new connection: " << strerror(errno) << std::endl;
            return (-1);
        }
        std::cout << "New clientFd " << clientFd << " accepted on port: " << portFD << std::endl;
        fcntl(clientFd, F_SETFL, O_NONBLOCK);
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET; 
        ev.data.fd = clientFd;
        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, clientFd, &ev) == -1) 
        {
            std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
            close(clientFd);
            return (-1);
        }
        selector.getActiveClients().insert(clientFd);
        selector.getClientConfig()[clientFd] = this->getConfig();
    }
    return (0);
}

static int writeToBodyPipe(std::vector<char>& body, size_t contentLength, int pipeIn)
{
	int flags = fcntl(pipeIn, F_GETFL, 0);
	fcntl(pipeIn, F_SETFL, flags | O_NONBLOCK);

    //check for -1 or 0
    int characters = write(pipeIn, &body[0], contentLength);
    close(pipeIn);
    if (characters <= -1) 
        return -1;
    return 0;
}

void Server::readClientRequest(Selector& selector, int clientFD)
{
    size_t                      pos = std::string::npos;
    static const std::string    RequestHeaderEnding = "\r\n\r\n";
    static const std::string    ChunkEnding = "\r\n";

    std::time_t start_time = std::time(0);

    HttpRequest* incomingRequestHTTP = new HttpRequest();
    incomingRequestHTTP->setConfig(selector.getClientConfig()[clientFD]);
    while (pos == std::string::npos) 
    {
        //error checking if request is bad
        if (std::time(0) - start_time > CLIENT_TIMEOUT)
        {
            //not checking -1 or 0 because i will remove the client anyways
            send(clientFD, HttpRequest::requestTimeout().c_str(), HttpRequest::requestTimeout().size(), 0); 
            selector.removeClient(clientFD);
            delete incomingRequestHTTP;
            return;
        }
        if (selector.getRequests()[clientFD].size() == 6)
        {
            std::string request = selector.getRequests()[clientFD];
            if (request.find("GET") == std::string::npos 
                    && request.find("POST") == std::string::npos
                    && request.find("DELETE") == std::string::npos)
            {
                //not checking -1 or 0 because i will remove the client anyways
                send(clientFD, HttpRequest::badRequest().c_str(), HttpRequest::badRequest().size(), 0);
                selector.removeClient(clientFD);
                delete incomingRequestHTTP;
                return;
            }
        }
        char buffer[1];
        ssize_t count = recv(clientFD, buffer, sizeof(buffer), 0);
        if (count == -1) 
        {
            selector.removeClient(clientFD);
            delete incomingRequestHTTP;
            return; // not done reading everything yet, so remove client as per subject
        }
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
    }
    std::cout << selector.getRequests()[clientFD] << std::endl;
    std::string request = selector.getRequests()[clientFD].c_str();
    if (request.size() == 0)
    {
        selector.getRequests()[clientFD].erase();
        selector.getHTTPRequests().erase(clientFD);
        delete incomingRequestHTTP;
        return;
    }
    incomingRequestHTTP->setBuffer(request.c_str());
    selector.getHTTPRequests()[clientFD] = incomingRequestHTTP;
    
    //is there a body
    size_t contentLength = selector.getBodyContentLength(clientFD);
    if (contentLength != std::string::npos)
    {
        std::cout << "there's a body" << std::endl;
        std::vector<char> bodyBuffer(contentLength);
        int err = recv(clientFD, bodyBuffer.data(), contentLength, 0);
        if (err <= -1)
        {
            std::cout << "readClientRequest  err: " << err << std::endl;
            selector.removeClient(clientFD);       
            delete incomingRequestHTTP;
            return;
        }
        else if (err != 0)
        {
           int err = writeToBodyPipe(bodyBuffer, contentLength, incomingRequestHTTP->_bodyPipe[1]);
           if (err == -1)
           {
                selector.removeClient(clientFD);
                delete incomingRequestHTTP;
                return;
           }
        }
        incomingRequestHTTP->getBody().raw = std::string(bodyBuffer.begin(), bodyBuffer.end());
    }
    selector.setClientFdEvent(clientFD, WRITE);
    incomingRequestHTTP->handler(selector, clientFD);
}

void Server::sendResponse(Selector& selector, int client_socket)
{
    HttpRequest*    clientHTTP  = selector.getHTTPRequests()[client_socket];
    size_t          totalSize   = clientHTTP->getResponse().size();

    std::cout << "response\n" << clientHTTP->getResponse() << std::endl;

    std::vector<char> vec(clientHTTP->getResponse().begin(), clientHTTP->getResponse().end());
    bool connectionClosed = (clientHTTP->getResponse().find("Connection: close") != std::string::npos) ? true : false;

    int err = send(client_socket, &vec[0], totalSize, 0);
    if (err == -1 || connectionClosed == true)
    {
        selector.removeClient(client_socket);
        delete clientHTTP;
        return;
    }
    else 
    {
        delete clientHTTP;
        selector.getHTTPRequests().erase(client_socket);
    }
}


std::string	toString(size_t num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}


int Server::handleResponsePipe(Selector& selector, int eventFd) 
{
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead;

    cgiProcessInfo* cgiInfo = selector.getCgis()[eventFd];
    int clientFd = cgiInfo->_clientFd;

    if (selector.checkCgiStatus(cgiInfo) != 0)
    {
        //not checking -1 or 0 because i will remove the client anyways
        send(clientFd, HttpRequest::serverError().c_str(), HttpRequest::serverError().size(), 0);
        selector.removeClient(clientFd);
        selector.deleteCgi(cgiInfo);
        return -1;
    }

    bytesRead = read(eventFd, buffer, sizeof(buffer));
    std::cout << "bytesRead: " << bytesRead<< std::endl;
    if (bytesRead > 0)
        cgiInfo->_ScriptResponse.append(buffer, bytesRead);
    else if (bytesRead == 0) 
    {
        if (this->sendCGIResponse(cgiInfo) == -1)
            selector.removeClient(clientFd);
        selector.deleteCgi(cgiInfo);
        std::memset(buffer, 0, sizeof(buffer));
        return (-1);
    } 
    else 
    {
        perror("handleResponsePipe: read");
        selector.deleteCgi(cgiInfo);
        selector.removeClient(clientFd);
        return -1;
    }
    return 0;
}

int Server::sendCGIResponse(cgiProcessInfo* cgiInfo)
{
    std::string statusLine  = "HTTP/1.1 200 OK\r\n";
    std::string headers     = "Server: webserver/0.42\r\n";
    std::stringstream contentLength;
    std::stringstream bodyStream(cgiInfo->_ScriptResponse);

    size_t found = cgiInfo->_ScriptResponse.find("Content-Type:");
    if (found != std::string::npos)
    {
        std::string line;
        std::getline(bodyStream, line);
        headers += line + "\r\n";
        std::getline(bodyStream, line);
    }
    else 
    {
        headers += "Content-Type: text/html\r\n";
    }
    std::stringstream tmp;
    tmp << bodyStream.rdbuf();
    contentLength << "Content-Length: " << tmp.str().size() << "\r\n";
    headers += contentLength.str();
    headers += "\r\n";

    std::string response = statusLine + headers + tmp.str();
    std::vector<char> vec(response.begin(), response.end());
    int err = send(cgiInfo->_clientFd, &vec[0], response.size(), 0);
    if (err <= -1) return -1;
    return 0;

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

