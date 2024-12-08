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

std::string Server::readBodyRequest(size_t contentLength, int clientFd)
{
    char buffer[contentLength];
    ssize_t count = recv(clientFd, buffer, sizeof(buffer), 0);
    if (count == -1)
        return std::string();
    
    return (buffer);
}


void Server::readClientRequest(Selector& selector, int clientFD)
{
    size_t                      pos = std::string::npos;
    static const std::string    RequestHeaderEnding = "\r\n\r\n";
    static const std::string    ChunkEnding = "\r\n";

    while (pos == std::string::npos) 
    {
        char buffer[1];
        ssize_t count = recv(clientFD, buffer, sizeof(buffer), 0);
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
    }
    //is there a body
    size_t contentLength = selector.getBodyContentLength(clientFD);
    if (contentLength != std::string::npos)
    {
    

        char *bodyBuffer = new char[contentLength + 1];
        recv(clientFD, bodyBuffer, contentLength, 0);
        bodyBuffer[contentLength] = '\0';
        std::cout << bodyBuffer << std::endl; 
        std::string response(bodyBuffer);
        selector.getRequests()[clientFD] += response;
        delete[] bodyBuffer;
    }
    
    // char buffer[4096];
    // headers
    //  Content-Length: number
    //  /r\n\r\n
    
    // char *bodyBuffer = new char[cone];

    selector.setClientFdEvent(clientFD, WRITE);
}

static void writeToBodyPipe(std::string request, int pipeIn)
{
    static const std::string    RequestHeaderEnding = "\r\n\r\n";

    size_t start = request.find(RequestHeaderEnding);
    std::string body = request.substr(start + RequestHeaderEnding.size());
    write(pipeIn, body.c_str(), body.size());
    close(pipeIn);
}

int Server::sendResponse(Selector& selector, int client_socket, std::string request)
{
    const char* buffer = request.c_str();
    if (request.size() == 0)
    {
        selector.getRequests()[client_socket].erase();
        return (-1);
    }

    HttpRequest* incomingRequestHTTP = new HttpRequest();
    incomingRequestHTTP->setConfig(selector.getClientConfig()[client_socket]);
    incomingRequestHTTP->setBuffer(buffer);
    writeToBodyPipe(request, incomingRequestHTTP->_bodyPipe[1]);
    std::string response = incomingRequestHTTP->handler(selector, client_socket);
    int sent_bytes = send(client_socket, response.c_str(), response.size(), 0);
    if (sent_bytes < 0) 
    {
        selector.removeClient(client_socket);
        delete incomingRequestHTTP;
        return (-1);
    }
    delete incomingRequestHTTP;
    selector.getRequests().erase(client_socket);
    selector.setClientFdEvent(client_socket, READ);
    return (0);
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
    ssize_t bytesRead;
    //do i need to malloc memory here for the cgi process

    cgiProcessInfo* cgiInfo = selector.getCgis()[eventFd];
    int clientFd = cgiInfo->_clientFd;

    // Read and send data incrementally
    /*while ((bytesRead = read(eventFd, buffer, sizeof(buffer))) > 0) */
    /*{*/
    /*        ssize_t bytesSent = 0;*/
    /*        while (bytesSent < bytesRead) */
    /*        {*/
    /*            ssize_t sent = send(clientFd, buffer + bytesSent, bytesRead - bytesSent, 0);*/
    /*            std::cout << "bytesRead: " << bytesRead << std::endl;*/
    /*            std::cout << "sent: " << sent << std::endl;*/
    /*            if (sent == -1) */
    /*            {*/
    /*                if (errno == EAGAIN || errno == EWOULDBLOCK) */
    /*                    break; // Buffer is full, stop*/
    /*                perror("send");*/
    /*                removeClient(selector, clientFd);*/
    /*                return -1;*/
    /*            }*/
    /*            bytesSent += sent;*/
    /*    }*/
    /*}*/

    bytesRead = read(eventFd, buffer, sizeof(buffer));
    std::cout << "buffer size: " << bytesRead << std::endl;
    std::cout << "buffer: " << buffer << std::endl;
    if (bytesRead > 0)
        cgiInfo->_ScriptResponse += buffer;
    else if (bytesRead == 0) 
    {
        this->sendCGIResponse(cgiInfo);
        selector.deleteCgi(cgiInfo);
        std::memset(buffer, 0, sizeof(buffer));
        return (-1);
    } 
    else 
    {
        //remove from epoll instance
        perror("handleResponsePipe: read");
        selector.deleteCgi(cgiInfo);
        selector.removeClient(clientFd);
        return -1;
    }
    return 0;
}

void Server::sendCGIResponse(cgiProcessInfo* cgiInfo)
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
        std::getline(bodyStream, line); // skipping newline
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

    //TODO: check for fails in send
    send(cgiInfo->_clientFd, response.c_str(), response.size(), 0);
}

/*int Server::handleResponsePipe(Selector& selector, int pipeFd) */
/*{*/
/*    char buffer[4096];*/
/*    ssize_t bytesRead;*/
/*    cgiProcessInfo& cgiInfo = selector.getCgiProcessInfo();*/
/*    int clientFd = cgiInfo._clientFd;*/
/**/
/*    // Determine if the response should be chunked or unchunked*/
/*    std::string request = selector.getRequests()[clientFd];*/
/*    //bool isChunked = request.isChunked(); // Assume isChunked() checks the transfer type*/
/**/
/*    // Read and send data incrementally*/
/*    while ((bytesRead = read(pipeFd, buffer, sizeof(buffer))) > 0) */
/*    {*/
/*        if (isChunked) */
/*        {*/
/*            // Format and send chunked data*/
/*            std::ostringstream chunk;*/
/*            chunk << std::hex << bytesRead << "\r\n"; // Chunk size in hex*/
/*            chunk.write(buffer, bytesRead);          // Chunk data*/
/*            chunk << "\r\n";                         // End of chunk*/
/**/
/*            std::string chunkStr = chunk.str();*/
/*            ssize_t bytesSent = 0;*/
/*            while (bytesSent < chunkStr.size()) {*/
/*                ssize_t sent = send(clientFd, chunkStr.c_str() + bytesSent, chunkStr.size() - bytesSent, 0);*/
/*                if (sent == -1) {*/
/*                    if (errno == EAGAIN || errno == EWOULDBLOCK) break; // Buffer is full, stop*/
/*                    perror("send");*/
/*                    close(pipeFd);*/
/*                    close(clientFd);*/
/*                    selector.getRequests().erase(clientFd);*/
/*                    return -1;*/
/*                }*/
/*                bytesSent += sent;*/
/*            }*/
/*        }*/
/*        else */
/*        {*/
/*            // Send unchunked data*/
/*            ssize_t bytesSent = 0;*/
/*            while (bytesSent < bytesRead) */
/*            {*/
/*                ssize_t sent = send(clientFd, buffer + bytesSent, bytesRead - bytesSent, 0);*/
/*                if (sent == -1) */
/*                {*/
/*                    if (errno == EAGAIN || errno == EWOULDBLOCK) */
/*                        break; // Buffer is full, stop*/
/*                    perror("send");*/
/*                    //removeClient(selector, pipeFd);*/
/*                    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, pipeFd, NULL) == -1)*/
/*                    {*/
/*                        std::cout << "problem when trying to remove pipe: " << pipeFd << std::endl;*/
/*                        exit(1);*/
/*                    }*/
/*                    removeClient(selector, clientFd);*/
/*                    return -1;*/
/*                }*/
/*                bytesSent += sent;*/
/*            }*/
/*        }*/
/*    }*/
/**/
/*    if (bytesRead == 0) */
/*    {*/
/*        // EOF: End of response*/
/*        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, pipeFd, NULL) == -1) {*/
/*            perror("epoll_ctl: remove pipeFd");*/
/*            std::cout << "Failed to remove pipeFd: " << pipeFd << ", errno: " << errno << std::endl;*/
/*            exit(1);*/
/*        }*/
/*        close(pipeFd);*/
/**/
/*        if (isChunked) */
/*        {*/
/*            // Send the final chunk for chunked transfer encoding*/
/*            std::string finalChunk = "0\r\n\r\n";*/
/*            send(clientFd, finalChunk.c_str(), finalChunk.size(), 0);*/
/*        }*/
/**/
/*        // Handle connection persistence*/
/*        selector.setClientFdEvent(clientFd, READ);*/
/*    } */
/*    else if (bytesRead == -1)*/
/*    {*/
/*        //remove from epoll instance*/
/*        perror("handleResponsePipe: read");*/
/*        std::cout << "pipeFd: " << pipeFd << std::endl;*/
/*        std::cout << "clientFd: " << clientFd << std::endl;*/
/*        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, pipeFd, NULL) == -1) {*/
/*            perror("epoll_ctl: remove pipeFd");*/
/*            std::cout << "Failed to remove pipeFd: " << pipeFd << ", errno: " << errno << std::endl;*/
/*            exit(1);*/
/*        }*/
/*        close(pipeFd);*/
/*        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, pipeFd, NULL) == -1)*/
/*        {*/
/*            std::cout << "problem when trying to remove pipe: " << pipeFd << std::endl;*/
/*            exit(1);*/
/*        }*/
/**/
/*        //removeClient(selector, pipeFd);*/
/*        removeClient(selector, clientFd);*/
/*        return -1;*/
/*    }*/
/**/
/*    return 0;*/
/*}*/
/**/

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


