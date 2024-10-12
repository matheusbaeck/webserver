#include "Selector.hpp"

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

	oss() << "addSocket: Succlearcessfully added socket fd " << socket_fd << " to epoll";
	LogMessage(DEBUG);
}

void Selector::processEvents(std::queue<Request>& requestQueue)
{
    m_nfds = epoll_wait(m_epollfd, m_events, MAX_EVENTS, TIME_OUT);
    if (m_nfds == -1) {
        oss() << "epoll_wait failed: " << strerror(errno);
        LogMessage(ERROR);
        return;
    }

    for (int n = 0; n < m_nfds; ++n) {
        int event_fd = m_events[n].data.fd;

        Worker* worker = getWorkerByFd(event_fd);
        if (!worker) {
            oss() << "processEvents: No worker found for fd " << event_fd;
            LogMessage(ERROR);
            continue;
        }

        if (m_events[n].events & EPOLLIN) {
            if (event_fd == worker->sock()) {
                // This is a listening socket, accept the new connection
                int client_fd = worker->accept_connection();
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
            } else {
                // This is a client socket, create a request
                Request request(event_fd, worker->port());
                requestQueue.push(request);

                oss() << "processEvents: Added request for fd " << event_fd << " to queue";
                LogMessage(INFO);
            }
        }
        
        if (m_events[n].events & (EPOLLERR | EPOLLHUP)) {
            oss() << "Error or hangup on fd " << event_fd << ": " << strerror(errno);
            LogMessage(ERROR);
            epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event_fd, NULL);
            close(event_fd);
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
