#include "Selector.hpp"
#include <cstring>
#include <map>

Selector Selector::selector;

//TODO: replace all logs with std::cerr or std::cout
//TODO: what happens if epollfd fails
Selector::Selector(void) 
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

std::map<int, cgiProcessInfo*>& Selector::getCgiProcessInfo()
{
    return _cgiProcesses;
}

void Selector::addCgi(int pipeFd, cgiProcessInfo* CgiProcess)
{
    _cgiProcesses[pipeFd] = CgiProcess;
}

void Selector::deleteCgi(cgiProcessInfo* CgiProcess)
{
    std::cout << "Deleting CGI Process for response pipe: " << CgiProcess->_responsePipe << std::endl;

    // Remove from epoll instance
    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, CgiProcess->_responsePipe, NULL) == -1) {
        perror("epoll_ctl: remove eventFd");
        std::cerr << "Failed to remove eventFd: " << CgiProcess->_responsePipe << ", errno: " << errno << std::endl;
        exit(1);
    }

    // Remove from _cgiProcesses
    size_t erased = _cgiProcesses.erase(CgiProcess->_responsePipe);
    if (erased == 0) {
        std::cerr << "Warning: CGI process not found in _cgiProcesses!" << std::endl;
    }

    // Close response pipe
    close(CgiProcess->_responsePipe);
    std::cout << "Closed response pipe: " << CgiProcess->_responsePipe << std::endl;

    // Clear and delete CGI process
    CgiProcess->_path.clear();
    CgiProcess->_ScriptResponse.clear();
    delete CgiProcess;
    std::cout << "Deleted CGI Process." << std::endl;
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

bool Selector::isResponsePipe(int event_fd) const
{
    if (_cgiProcesses.size())
    {
        std::map<int, cgiProcessInfo*>::const_iterator it = _cgiProcesses.begin();
        while (it != _cgiProcesses.end())
        {
            if (event_fd == it->second->_responsePipe)
                return true;
            it++;
        }
    }
    return (false);
}

void Selector::setClientFdEvent(int event_fd, int action)
{
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
                int event_fd = _events[n].data.fd;

                // Reading requests
                if (_events[n].events & EPOLLIN) 
                {
                    if (isServerSocket(event_fd, serverSocket)) 
                    {
                        std::cout << "event_Fd: " << event_fd << std::endl;
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
                        int err = server->handleResponsePipe(*this, event_fd); // Handle CGI output
                        if (err == -1)
                            return;
                    }
                }

                // Answering clients
                if (_events[n].events & EPOLLOUT) 
                {
                    err = server->sendResponse(*this, event_fd, _requests[event_fd]);
                    if (err == -1)
                    {
                        _requests.erase(event_fd);
                        continue;
                    }
                    else 
                        return;
                }
                if (_events[n].events & (EPOLLERR | EPOLLHUP)) 
                {
                    //kill CGI?
                    std::cerr << "Error on FD " << _events[n].data.fd << ": " << strerror(errno) << std::endl;
                    epoll_ctl(this->getEpollFD(), EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                    _activeClients.erase(_events[n].data.fd);
                    _clientConfig.erase(_events[n].data.fd);
                    _requests.erase(_events[n].data.fd);
                    close(_events[n].data.fd);
                }
            }
        }
    }
}



