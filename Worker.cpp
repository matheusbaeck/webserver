#include "Worker.hpp"

Worker::Worker( void )
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(SERVER_PORT);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_serv_socket = create_server_socket();
}

Worker::Worker( int port )
{
	this->m_addr.sin_family = AF_INET;
	this->m_addr.sin_port = htons(port);
	this->m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_serv_socket = create_server_socket();
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
	// AHMED START
	int reuse = 1;
	if (setsockopt(this->m_serv_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt");
		return -1;
	}
	// AHMED END
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

void	hellow_world(int fd)
{
	char buffer[BUFFERSIZE];
	ssize_t bytes_read;

	bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0)
	{
		if (bytes_read == 0)
		{
			close(fd);
		}
		else
		{
			perror("read");
		}
		return ;
	}
	const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 12\r\n"
                           "\r\n"
                           "Hello World!";
    write(fd, response, strlen(response));
	close(fd);
}


void	Worker::run( void )
{
	struct epoll_event	ev, events[MAX_EVENTS];
	int					conn_sock, nfds, epollfd;
	socklen_t			addrlen;

	addrlen = sizeof(*this->addr());
	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = this->m_serv_socket;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->m_serv_socket, &ev) == -1) {
		perror("epoll_ctl: serv_socket");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, TIME_OUT);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == this->m_serv_socket) {
				conn_sock = accept(this->m_serv_socket, this->addr(), &addrlen);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;

				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				printf("server: handeling request\n");
				hellow_world(conn_sock);
				//req_handler(events[n].data.fd);
			}
		}
	}
}
