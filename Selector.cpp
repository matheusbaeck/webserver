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

void	Selector::processEvents(std::queue<Request>& requestQueue)
{
	m_nfds = epoll_wait(m_epollfd, m_events, MAX_EVENTS, TIME_OUT);
	if (m_nfds == -1) {
		oss() << "epoll_wait failed";
		LogMessage(ERROR);
		return;
	}

	for (int n = 0; n < m_nfds; ++n) {
		int event_fd = m_events[n].data.fd;

		Worker* worker = getWorkerByFd(event_fd);
		if (!worker) {
			LogMessage(ERROR, "processEvents: No worker found for fd");
			continue;
		}

		if (m_events[n].events & EPOLLIN) {
			Request request(event_fd, worker->port());
			requestQueue.push(request);

			oss() << "processEvents: Added request for fd " << event_fd << " to queue";
			LogMessage(INFO);
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
