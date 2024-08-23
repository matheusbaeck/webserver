#include "Worker.hpp"

Worker::Worker( void ) : m_serv_port(SERVER_PORT)
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(SERVER_PORT);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
	this->m_serv_socket = create_server_socket(); /* handle error*/

	this->m_epollfd = epoll_create1(0);
	if (this->m_epollfd == -1) {
		perror("epoll_create1");
		/* handle error */
	}
	this->m_ev.events = EPOLLIN | EPOLLET;
	this->m_ev.data.fd = this->m_serv_socket;
	if (epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, this->m_serv_socket, &this->m_ev) == -1) {
		perror("epoll_ctl: serv_socket");
		/* handle error */
	}
}

Worker::Worker( int port ) : m_serv_port(port)
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(port);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addrlen = sizeof(*this->addr());
	this->m_serv_socket = create_server_socket();

	this->m_epollfd = epoll_create1(0);
	if (this->m_epollfd == -1) {
		perror("epoll_create1");
		/* handle error */
	}
	this->m_ev.events = EPOLLIN | EPOLLET;
	this->m_ev.data.fd = this->m_serv_socket;
	if (epoll_ctl(this->m_epollfd, EPOLL_CTL_ADD, this->m_serv_socket, &this->m_ev) == -1) {
		perror("epoll_ctl: serv_socket");
		/* handle error */
	}
	this->m_nfds = epoll_wait(this->m_epollfd, this->m_events, MAX_EVENTS, TIME_OUT);
	if (this->m_nfds == -1) {
		perror("epoll_wait");
		/* handle error */
	}
}

Worker::~Worker()
{
	close(this->m_serv_socket);
}

sockaddr	*Worker::addr( void ) { return ((sockaddr *)&this->m_addr); }

int Worker::create_server_socket( void )
{
	int addrlen = sizeof(*this->addr());
	this->m_serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->m_serv_socket < 0)
	{
		perror("socket");
		/* handle error */
	}
	int reuse = 1;
	if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt");
		return -1;
	}
	//bzero(this->m_addr, addrlen);
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_addr.sin_port = htons(SERVER_PORT);
	if (bind(this->m_serv_socket, this->addr(), addrlen) == -1)
	{
		perror("bind");
		/* handle error */
	}
	if (listen(this->m_serv_socket, BACKLOG) < 0)
	{
		perror("listen");
		/* handle error */
	}
	return (this->m_serv_socket);
}

static void	setnonblocking(int conn_sock)
{
	int	flags;

	flags = fcntl(conn_sock, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL)");
		return;
	}
}

void	Worker::run( std::queue<Request> &requests )
{
	int conn_sock;

	this->m_nfds = epoll_wait(this->m_epollfd, this->m_events, MAX_EVENTS, TIME_OUT);
	if (this->m_nfds == -1) {
		perror("epoll_wait");
		/* handle error */
	}

	for (int n = 0; n < this->m_nfds; ++n) {
		if (this->m_events[n].data.fd == this->m_serv_socket) {
			conn_sock = accept(this->m_serv_socket, this->addr(), &this->m_addrlen);
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
			std::cout << "request add to queue" << std::endl;
			requests.push(this->m_events[n].data.fd, this->m_serv_port);
		}
	}
}
