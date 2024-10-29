#include "Selector.hpp"
#include "HttpRequest.hpp"
#include <cstring>


Selector Selector::selector;

//TODO: replace all logs with std::cerr or std::cout
Selector::Selector( void )
{
	this->m_epollfd = epoll_create(1);
	if (this->m_epollfd == -1)
    {
        std::cerr << "Failed to create epoll_instance: " << strerror(errno) << std::endl;
	}
}

Selector::~Selector( void )
{
	if (this->m_epollfd != -1)
		close(this->m_epollfd);
}

void Selector::addSocket(const Server *server)
{
	int socket_fd = server->sock();
    std::cout << "addSocket: Adding socket fd " << socket_fd << " to epoll instance" << std::endl;
	m_ev.events = EPOLLIN;
	m_ev.data.fd = socket_fd;

	if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, socket_fd, &m_ev) == -1) 
    {
        std::cerr << "addSocket: Failed to add socket fd " << socket_fd << " to epoll: " << strerror(errno) << std::endl;
		return;
	}
    std::cout << "addSocket: Successfully added socket fd " << socket_fd << " to epoll" << std::endl;
    std::cout << std::endl;
}

void Selector::processEvents(std::vector<Server*>& servers )
{
    m_nfds = epoll_wait(m_epollfd, m_events, MAX_EVENTS, TIME_OUT);
    if (m_nfds == 0)
        return;
    if (m_nfds == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }
    for (size_t i = 0; i < servers.size(); i += 1)
    {
        for (int n = 0; n < m_nfds; ++n)
        {
            //int  client_fd = m_events[n].data.fd;
            if (m_events[n].events & EPOLLIN)
            {
                HttpRequest httpRequest;
                int	client_fd;

                if ( m_events[n].data.fd == servers[i]->sock())
                {
                    httpRequest.setConfig(servers[i]->getConfig());
                    client_fd = accept(servers[i]->sock(), NULL, NULL);
                    std::cout << "Accepted on port: " << servers[i]->port() << " | server: " << &servers[i] << std::endl;
                    std::cout << "client_fd: " << client_fd << " connected" << std::endl;

                    if (client_fd < 0)
                    {
                        std::cerr << "Failed to accept new connection: " << strerror(errno) << std::endl;
                        continue;
                    }
                    // Add the new client socket to epoll
                    epoll_event ev;
                    ev.events = EPOLLIN; //| EPOLLET;  // Edge-triggered mode
                    ev.data.fd = client_fd;
                    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1) 
                    {
                        std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
                        close(client_fd);
                        continue;
                    }

                }
                else
                {
                    char buffer[1024] = {0};
                    int received_bytes = recv(m_events[n].data.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                    if (received_bytes == 0)
                    {
                        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_events[n].data.fd, NULL);
                        close(m_events[n].data.fd);
                    }
                    if (received_bytes < 0)
                        continue;

                    httpRequest.setBuffer(buffer);
                    std::string response = httpRequest.handler();

                    int sent_bytes = send(m_events[n].data.fd, response.c_str(), response.size(), 0);
                    if (sent_bytes < 0) 
                    {
                        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_events[n].data.fd, NULL);
                        close(m_events[n].data.fd);
                        continue;
                    }

                    // TODO: handle if send fails and close connection after handling all requests.
                    if (m_events[n].events & (EPOLLERR| EPOLLHUP))
                    {
                        std::cerr << "Error on fd " <<  m_events[n].data.fd << ": " << strerror(errno) << std::endl;
                        epoll_ctl(m_epollfd, EPOLL_CTL_DEL,  m_events[n].data.fd, NULL);
                        close( m_events[n].data.fd);
                    }
                }
            }

        }
    }
}

