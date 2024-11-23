#include "Selector.hpp"
#include "HttpRequest.hpp"
#include <cstring>
#include <map>

Selector Selector::selector;

//TODO: replace all logs with std::cerr or std::cout
//TODO: what happens if epollfd fails
Selector::Selector( void ) : _cgiInfo(-1, -1, -1, -1)
{
	this->_epollfd = epoll_create(1);
	if (this->_epollfd == -1)
    {
        std::cerr << "Failed to create epoll_instance: " << strerror(errno) << std::endl;
	}
}

Selector::~Selector( void )
{
	if (this->_epollfd != -1)
		close(this->_epollfd);
}

epoll_event* Selector::getEvents()
{
    return (this->_events);
}

std::set<int>& Selector::getActiveClients()
{
    return this->_activeClients;
}

cgiProcessInfo& Selector::getCgiProcessInfo()
{
    return this->_cgiInfo;
}

void Selector::setCgiProcessInfo(cgiProcessInfo& CgiProcess)
{
    _cgiInfo = CgiProcess;
}

std::map<int, ConfigServer>& Selector::getClientConfig()
{
    return (this->_clientConfig);
}

std::map<int, std::string>& Selector::getRequests()
{
    return (this->_requests);
}

int& Selector::getEpollFD()
{
    return (this->_epollfd);
}


void Selector::addSocket(const Server *server)
{
    for (size_t i = 0; i < server->getSockets().size(); i += 1)
    {
        int socket_fd = server->getSockets()[i];
        _ev.events = EPOLLIN | EPOLLET;
        _ev.data.fd = socket_fd;

        if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, socket_fd, &_ev) == -1) 
        {
            std::cerr << "addSocket: Failed to add socket fd " << socket_fd << " to epoll: " << strerror(errno) << std::endl;
            return;
        }

    }
}

bool Selector::isServerSocket(int fd, int serverSocket)
{

    return (fd == serverSocket); 
}

bool Selector::isClientFD(int fd)
{
    return (this->_activeClients.find(fd) != _activeClients.end());
}

/*int responseReadFd = _cgiInfo.responsePipe.first;*/
/*int responseWriteFd = _cgiInfo.responsePipe.second;*/
/*int statusReadFd = _cgiInfo.statusPipe.first;*/
/*int statusWriteFd = _cgiInfo.statusPipe.second;*/

bool Selector::isResponsePipe(int event_fd)
{
    if (_cgiInfo._clientFd != -1)
        return (event_fd == _cgiInfo._responsePipe);
    return (false);
}

void Selector::setClientFdEvent(Server* server, int event_fd, int action)
{
    (void)server;
    struct epoll_event info; 
    //action == READ ? info.events = EPOLLIN | EPOLLET : info.events = EPOLLOUT | EPOLLET;
    if (action == READ)
        info.events = EPOLLIN | EPOLLET; 
    else
        info.events = EPOLLOUT | EPOLLET; 

    info.data.fd = event_fd;
    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_MOD, event_fd, &info) == -1) 
    {
        std::cerr << "Failed to modify epoll event for FD " << event_fd 
            << ": " << strerror(errno) << std::endl;
        close(event_fd);
        //server->cleanUpClient(*this, event_fd);
    }
}

void Selector::processEvents(const std::vector<Server*>& servers )
{
    _eventCount = epoll_wait(_epollfd, _events, MAX_EVENTS, TIME_OUT); //timeout to calculate CGI timeout and potential kill
    if (_eventCount == 0)
        return;
    if (_eventCount == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }
    int err;
    for (size_t i = 0; i < servers.size(); i += 1)
    {
        Server *server = servers[i];
        for (size_t j = 0; j < server->getSockets().size(); ++j) 
        {
            int serverSocket    = server->getSockets()[j];
            int serverPort      = server->getPorts()[j];
            for (int n = 0; n < _eventCount; ++n) 
            {
                int event_fd = _events[i].data.fd;

                // Reading requests
                if (_events[i].events & EPOLLIN) 
                {
                    if (isServerSocket(event_fd, serverSocket)) 
                    {
                        err = server->acceptConnection(*this, serverSocket, serverPort); // Accept new client connection
                        if (err == -1)
                            continue;
                    }
                    else if (isClientFD(event_fd))
                    {
                        std::cout << "----------------------isClientFd" << std::endl;
                        server->readClientRequest(*this, event_fd); // Read client request
                        return;
                    }
                    else if (isResponsePipe(event_fd))
                    {
                        std::cout << "----------------------isResponsePipe" << std::endl;
                        server->handleResponsePipe(*this, event_fd); // Handle CGI output
                    }
                    /*else if (isStatusPipe(event_fd))*/
                    /*{*/
                    /*    handleStatusPipe(event_fd); // Handle CGI exit status*/
                    /*}*/
                }

                // Answering clients
                if (_events[i].events & EPOLLOUT) 
                {
                    /*ClientRequest* request = getRequest(event_fd);*/
                    err = server->sendResponse(*this, event_fd, _requests[event_fd]);
                    if (err == -1)
                    {
                        _requests.erase(event_fd);
                        continue;
                    }
                }
                if (_events[n].events & (EPOLLERR | EPOLLHUP)) 
                {
                    //kill CGI?
                    std::cerr << "Error on FD " << _events[n].data.fd << ": " << strerror(errno) << std::endl;
                    epoll_ctl(this->getEpollFD(), EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                    _activeClients.erase(_events[n].data.fd);
                    _clientConfig.erase(_events[n].data.fd);
                    close(_events[n].data.fd);
                }
            }
        }
    }
}



