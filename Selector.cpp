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

void Selector::addSocket(const Server *server)
{
	int socket_fd = server->getSock();
    std::cout << "addSocket: Adding socket fd " << socket_fd << " to epoll instance" << std::endl;
	_ev.events = EPOLLIN;
	_ev.data.fd = socket_fd;

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, socket_fd, &_ev) == -1) 
    {
        std::cerr << "addSocket: Failed to add socket fd " << socket_fd << " to epoll: " << strerror(errno) << std::endl;
		return;
	}
}

void Selector::processEvents(const std::vector<Server*>& servers )
{
    HttpRequest *incomingRequestHTTP;

    _eventCount = epoll_wait(_epollfd, _events, MAX_EVENTS, -1); //old timeout 200
    if (_eventCount == 0)
        return;
    if (_eventCount == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }
    for (size_t i = 0; i < servers.size(); i += 1)
    {
        for (int n = 0; n < _eventCount; ++n)
        {

            if (_events[n].events & EPOLLIN)
            {
                int	client_fd;
                if ( _events[n].data.fd == servers[i]->getSock())
                {

                    client_fd = accept(servers[i]->getSock(), NULL, NULL);
                    _clientConfig[client_fd] = servers[i]->getConfig();
                    incomingRequestHTTP = new HttpRequest();

                    std::cout << "incoming Request HTTP: " << incomingRequestHTTP << std::endl;
                    std::cout << "New client_fd " << client_fd << " accepted on port: " << servers[i]->getPorts() << std::endl;
                    if (client_fd < 0)
                    {
                        std::cerr << "Failed to accept new connection: " << strerror(errno) << std::endl;
                        continue;
                    }
                    // Add the new client socket to epoll
                    epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET; 
                    ev.data.fd = client_fd;
                    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1) 
                    {
                        std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
                        close(client_fd);
                        continue;
                    }
                }
                else 
                {
                    std::cout << "_events[n].data.fd: " << _events[n].data.fd <<  std::endl;
                    char buffer[1024] = {0};
                    int received_bytes = recv(_events[n].data.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                    std::cout << "bytes read: " << received_bytes  << " from fd " << _events[n].data.fd << std::endl;
                    if (received_bytes == 0)
                    {
                        epoll_ctl(_epollfd, EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                        _clientConfig.erase(_events[n].data.fd);
                        close(_events[n].data.fd);
                        continue;
                    }
                    if (received_bytes < 0)
                        continue;

                    if (_clientConfig.find(_events[n].data.fd) == _clientConfig.end())
                        exit(1);
                    incomingRequestHTTP->setConfig(_clientConfig[_events[n].data.fd]);
                    incomingRequestHTTP->setBuffer(buffer);
                    std::string response = incomingRequestHTTP->handler();
                    std::cout << "_events[n].data.fd: " << _events[n].data.fd << " for ports: " << _clientConfig[_events[n].data.fd].getPorts()[0] <<  std::endl;
                    std::cout << "RESPONSE---------------------------------------" << std::endl;
                    std::cout << "response: " << response << std::endl;
                    int sent_bytes = send(_events[n].data.fd, response.c_str(), response.size(), 0);
                    if (sent_bytes < 0) 
                    {
                        epoll_ctl(_epollfd, EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                        _clientConfig.erase(_events[n].data.fd);
                        delete incomingRequestHTTP;
                        close(_events[n].data.fd);
                        continue;
                    }

                    // TODO: handle if send fails and close connection after handling all requests.
                    if (_events[n].events & (EPOLLERR| EPOLLHUP))
                    {
                        std::cerr << "Error on fd " <<  _events[n].data.fd << ": " << strerror(errno) << std::endl;
                        epoll_ctl(_epollfd, EPOLL_CTL_DEL,  _events[n].data.fd, NULL);
                        _clientConfig.erase(_events[n].data.fd);
                        delete incomingRequestHTTP;
                        close( _events[n].data.fd);
                    }
                }
            }

        }
    }
}

