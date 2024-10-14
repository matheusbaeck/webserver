#include "Selector.hpp"
#include "ServerManager.hpp"

Selector Selector::selector;

Selector::Selector( void )
{
	this->m_epollfd = epoll_create1(0);
	if (this->m_epollfd == -1){
		LogMessage(ERROR, "epoll_create1");
	}
}

Selector::~Selector( void )
{
	if (this->m_epollfd != -1)
		close(this->m_epollfd);
}

void Selector::addSocket(const Worker &worker)
{
	int socket_fd = worker.sock();
	m_ev.events = EPOLLIN;
	m_ev.data.fd = socket_fd;

	oss() << "addSocket: Adding socket fd " << socket_fd << " to epoll instance";
	LogMessage(DEBUG);

	if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, socket_fd, &m_ev) == -1) {
		oss() << "addSocket: Failed to add socket fd " << socket_fd << " to epoll: " << strerror(errno);
		LogMessage(ERROR);
		return;
	}

	m_fd_to_worker_map[socket_fd] = const_cast<Worker*>(&worker);

	oss() << "addSocket: Successfully added socket fd " << socket_fd << " to epoll";
	LogMessage(DEBUG);
}


void Selector::executeMethod(char* buffer, int fd, Server& server)
{
    (void)server;
    HttpRequest httpRequest(buffer);
    httpRequest.setConfigFile(*ServerManager::getConfigFile());

    std::string response = httpRequest.handler();
    int sent_bytes = send(fd, response.c_str(), response.size(), 0);
    (void) sent_bytes;
}

void Selector::processEvents( Server & server )
{
    m_nfds = epoll_wait(m_epollfd, m_events, MAX_EVENTS, TIME_OUT);
    if (m_nfds == -1) {
        oss() << "epoll_wait failed: " << strerror(errno);
        LogMessage(ERROR);
        return;
    }
	for (int n = 0; n < m_nfds; ++n) 
	{
		int event_fd = m_events[n].data.fd;
		if (m_events[n].events & EPOLLIN) 
        {
			for (std::vector<Worker>::iterator it = server.workersBegin() ; it != server.workersEnd() ; ++it)
			{
				int	client_fd;
                if (event_fd == it->sock())
                {
                    client_fd = accept(it->sock(), NULL, NULL);
                    oss() << "client fd: " << client_fd << " connected" << std::endl;
                    LogMessage(INFO);

                    if (client_fd < 0) {
                        oss() << "Failed to accept new connection: " << strerror(errno);
                        LogMessage(ERROR);
                        continue;
                    }
                    // Add the new client socket to epoll
                    epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;  // Edge-triggered mode
                    ev.data.fd = client_fd;
                    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                        oss() << "Failed to add client socket to epoll: " << strerror(errno);
                        LogMessage(ERROR);
                        close(client_fd);
                        continue;
                    }

                    oss() << "Accepted new connection on fd " << client_fd;
                    LogMessage(INFO);
                } 
                else 
                {
                    // This is a client socket, create a request
                    char buffer[1024] = {0};

                    int received_bytes = recv(m_events[n].data.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                    if (received_bytes == 0)
                    {
                        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_events[n].data.fd, m_events);
                        close(m_events[n].data.fd);
                        oss() << "Client disconnected " << m_events[n].data.fd;
                        LogMessage(INFO);
                    }
                    if (received_bytes < 0)
                    {
                        oss() << "Error in received bytes:" <<strerror(errno);
                        LogMessage(ERROR);
                        continue;
                    }
                    selector.executeMethod(buffer, m_events[n].data.fd, server);
                }

                // TODO: handle if send fails and close connection after handling all requests.
                if (m_events[n].events & (EPOLLERR| EPOLLHUP)) 
                {
                    oss() << "Error on fd " << event_fd << ": " << strerror(errno);
                    LogMessage(ERROR);
                    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event_fd, NULL);
                    close(event_fd);
                }
            }
		}
		
	}
}
Worker*	Selector::getWorkerByFd(int fd) const
{
	std::map<int, Worker*>::const_iterator it = m_fd_to_worker_map.find(fd);
	if (it != m_fd_to_worker_map.end()) {
		return it->second;
	}
	return NULL;
}

void	Selector::LogMessage(int logLevel, const std::string& message, std::exception* ex)
{
	logger->logMessage(this, logLevel, message, ex);
}

void	Selector::LogMessage(int logLevel, std::exception* ex)
{
	logger->logMessage(this, logLevel, m_oss.str(), ex);
}

std::string	Selector::GetType() const
{
	return "Selector";
}
