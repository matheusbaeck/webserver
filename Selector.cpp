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

int& Selector::getEpollFD()
{
    return (this->_epollfd);
}


void Selector::addSocket(const Server *server)
{
    for (size_t i = 0; i < server->getSockets().size(); i += 1)
    {
        int socket_fd = server->getSockets()[i];
        _ev.events = EPOLLIN;
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
    _eventCount = epoll_wait(_epollfd, _events, MAX_EVENTS, 200); //old timeout 200
    if (_eventCount == 0)
        return;
    if (_eventCount == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }
    for (size_t i = 0; i < servers.size(); i += 1)
    {
        for (size_t j = 0; j < servers[i]->getSockets().size(); j += 1)
        {
            Server *server = servers[i];
            for (int n = 0; n < _eventCount; ++n)
            {
                
                if (_events[n].events & EPOLLIN)
                {
                    if ( _events[n].data.fd == server->getSockets()[j])
                    {
                        int err = server->acceptClient(selector, server->getSockets()[j], server->getPorts()[j]);
                        if (err == -1)
                            continue;
                    }
                    else 
                    {
                        int err = server->handleHTTPRequest(*this, _events[n].data.fd);
                        if (err == -1) 
                            continue;
                    }
                }

            }

        }
    }
}

