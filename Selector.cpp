#include "Selector.hpp"

Selector Selector::selector;

Selector::Selector( void )
{
	this->m_epollfd = epoll_create1(0);
	if (this->m_epollfd == -1) {
		LogMessage(ERROR, "epoll_create1");
	}
}

Selector::~Selector( void )
{
	if (this->m_epollfd != -1)
		close(this->m_epollfd);
}

void Selector::addSocket( int listen_sock )
{
	this->m_ev.events = EPOLLIN | EPOLLET;
	this->m_ev.data.fd = listen_sock;
	if (epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, listen_sock, &this->m_ev) == -1) {
		LogMessage(ERROR, "epoll_ctl: serv_socket");
	}
}

void	Selector::setnonblocking(int conn_sock)
{
	int	flags;

	flags = fcntl(conn_sock, F_GETFL, 0);
	if (flags == -1) {
		LogMessage(ERROR, "fcntl(F_GETFL)");
		return;
	}
	if (fcntl(conn_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		LogMessage(ERROR, "fcntl(F_SETFL)");

	}
}

void	Selector::putEventsToQ( const Worker &worker , std::queue<Request> &requests )
{
	int conn_sock;
	socklen_t len;
	
	oss() << "putEventsToQ:" << worker << "port " << worker.port() << " on socket " << worker.sock();
	LogMessage(TRACE);
	len = worker.addrlen();
	this->m_nfds = epoll_wait(this->m_epollfd, this->m_events, MAX_EVENTS, TIME_OUT);
	if (this->m_nfds == -1) {
		perror("epoll_wait");
		/* handle error */
	}
	// else {
	// 	std::cout << "epoll_wait returned " << this->m_nfds << " events." << std::endl;
	// }

	for (int n = 0; n < this->m_nfds; ++n) {
		if (this->m_events[n].data.fd == worker.sock()) {
			conn_sock = accept(worker.sock(), worker.addr(), &len);
			if (conn_sock == -1) {
				perror("accept");
				continue;
			}
			setnonblocking(conn_sock);
			this->m_ev.events = EPOLLIN | EPOLLET;
			this->m_ev.data.fd = conn_sock;

			if (epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, conn_sock, &this->m_ev) == -1) {
				perror("epoll_ctl: conn_sock");
				close(conn_sock);
				continue;
			}
		} else {
			LogMessage(INFO, "request add to queue");
			requests.push(Request(this->m_events[n].data.fd, worker.port()));
		}
	}
}
