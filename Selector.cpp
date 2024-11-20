#include "Selector.hpp"
#include "HttpRequest.hpp"
#include <cstring>
#include <map>

Selector Selector::selector;

//TODO: replace all logs with std::cerr or std::cout
//TODO: what happens if epollfd fails
Selector::Selector( void )
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

void Selector::processEvents(const std::vector<Server*>& servers )
{
    _eventCount = epoll_wait(_epollfd, _events, MAX_EVENTS, -1); //old timeout 200
    if (_eventCount == 0)
        return;
    if (_eventCount == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }

    static const std::string    RequestHeaderEnding = "\r\n\r\n";
    for (size_t i = 0; i < servers.size(); i += 1)
    {
        Server *server = servers[i];
        for (int n = 0; n < _eventCount; ++n) 
        {
            if (_events[n].events & EPOLLIN) 
            {
                bool is_server_socket = false;
                for (size_t j = 0; j < server->getSockets().size(); ++j) 
                {
                    if (_events[n].data.fd == server->getSockets()[j]) 
                    {
                        is_server_socket = true;
                        int err = server->acceptClient(selector, server->getSockets()[j], server->getPorts()[j]);
                        if (err == -1) 
                        {
                            std::cerr << "Error accepting client!" << std::endl;
                        }
                        break;
                    }
                }
                if (!is_server_socket) 
                {
                    size_t pos = std::string::npos;
                    while (pos == std::string::npos) 
                    {
                        char buffer[1024];
                        ssize_t count = read(_events[n].data.fd, buffer, sizeof(buffer));
                        if (count == -1) 
                            return; // not done reading everything yet, so return
                        if (count <= 0) 
                        { 
                            epoll_ctl(_epollfd, EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                            _clientConfig.erase(_events[n].data.fd);
                            close(_events[n].data.fd); 
                            break; 
                        } // passes? then bail on connection, as it's borked
                        _requests[_events[n].data.fd] += std::string(buffer, buffer + count);
                        pos = _requests[_events[n].data.fd].find(RequestHeaderEnding);
                        if (pos == std::string::npos) 
                            continue;
                        struct epoll_event info; 
                        info.events = EPOLLOUT | EPOLLET; 
                        info.data.fd = _events[n].data.fd;
                        epoll_ctl(_epollfd, EPOLL_CTL_MOD, _events[n].data.fd, &info); // MOD == modify existing event
                    }

                }
            }
            else if (_events[n].events & EPOLLOUT) 
            {
                std::cout << "Handling EPOLLOUT event for FD: " << _events[n].data.fd << std::endl;
                int err = server->handleHTTPRequest(*this, _events[n].data.fd, _requests[_events[n].data.fd]);
                if (err == -1)
                    continue;
                _requests.erase(_events[n].data.fd);
            }
            else if (_events[n].events & (EPOLLERR | EPOLLHUP)) 
            {
                //kill CGI?
                std::cerr << "Error on FD " << _events[n].data.fd << ": " << strerror(errno) << std::endl;
                epoll_ctl(this->getEpollFD(), EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                close(_events[n].data.fd);
            }
        }
    }
}
