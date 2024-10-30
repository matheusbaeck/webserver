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
    std::cout << "addSocket: Successfully added socket fd " << socket_fd << " to epoll" << std::endl;
    std::cout << std::endl;
}

void Selector::processEvents(const std::vector<Server*>& servers )
{
//    std::map<int, ConfigServer> fd_2_serverConfig;
    _nfds = epoll_wait(_epollfd, _events, MAX_EVENTS, TIME_OUT);
    if (_nfds == 0)
        return;
    if (_nfds == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }
    for (size_t i = 0; i < servers.size(); i += 1)
    {
        for (int n = 0; n < _nfds; ++n)
        {
            if (_events[n].events & EPOLLIN)
            {
              //  HttpRequest *httpRequest = new HttpRequest();
                HttpRequest incomingRequestHTTP;
                std::cout << "incoming Request HTTP: " << &incomingRequestHTTP << std::endl;
                int	client_fd;

                if ( _events[n].data.fd == servers[i]->getSock())
                {

                   // fd_2_serverConfig[_events[n].data.fd] = servers[i]->getConfig();
                    client_fd = accept(servers[i]->getSock(), NULL, NULL);
                    incomingRequestHTTP.setConfig(servers[i]->getConfig());
                    std::cout << "Accepted on port: " << servers[i]->getPorts() << std::endl;
                    std::cout << "client_fd: " << client_fd << " connected" << std::endl;

                    if (client_fd < 0)
                    {
                        std::cerr << "Failed to accept new connection: " << strerror(errno) << std::endl;
                        continue;
                    }
                    // Add the new client socket to epoll
                    epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;  // wtf is Edge-triggered mode
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
                    char buffer[1024] = {0};
                    int received_bytes = recv(_events[n].data.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                    if (received_bytes == 0)
                    {
                        epoll_ctl(_epollfd, EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                        close(_events[n].data.fd);
                    }
                    if (received_bytes < 0)
                        continue;

                    incomingRequestHTTP.setBuffer(buffer);
        /*            std::map<int, ConfigServer>::iterator it;
                    it = fd_2_serverConfig.find(_events[n].data.fd);
                    if (it != fd_2_serverConfig.end())
                    {
                        incomingRequestHTTP->setConfig(fd_2_serverConfig[_events[n].data.fd]);
                    }
          */        std::string response = incomingRequestHTTP.handler();
                    int sent_bytes = send(_events[n].data.fd, response.c_str(), response.size(), 0);
                    if (sent_bytes < 0) 
                    {
                        epoll_ctl(_epollfd, EPOLL_CTL_DEL, _events[n].data.fd, NULL);
                        close(_events[n].data.fd);
                        continue;
                    }

                    // TODO: handle if send fails and close connection after handling all requests.
                    if (_events[n].events & (EPOLLERR| EPOLLHUP))
                    {
                        std::cerr << "Error on fd " <<  _events[n].data.fd << ": " << strerror(errno) << std::endl;
                        epoll_ctl(_epollfd, EPOLL_CTL_DEL,  _events[n].data.fd, NULL);
                        close( _events[n].data.fd);
                    }
                }
             //   delete incomingRequestHTTP;
            }

        }
    }
}

