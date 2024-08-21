#include <arpa/inet.h> // inet_pton [convert string IP to binary]
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset, bzero
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
# include <sys/epoll.h>
#include <fcntl.h>

#define SERVER_PORT 8080
#define BUFFERSIZE 4096
#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT -1			//epoll_wait max time
#define BACKLOG 10			//listen param

void	setnonblocking(int conn_sock)
{
	int	flags;

	flags = fcntl(conn_sock, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL)");
		return;
	}
}

void	req_handler(int fd)
{
	char buffer[BUFFERSIZE];
	ssize_t bytes_read;

	bytes_read = read(fd, buffer, BUFFERSIZE - 1);
	if (bytes_read == -1)
	{
		perror("read");
		close(fd);
		return;
	}
	else if (bytes_read == 0)
	{
		printf("Connection closed by peer\n");
		close(fd);
		return;
	}
	buffer[bytes_read] = '\0';
	printf("Received: %s\n", buffer);
}

void	epoll_handler(int listen_sock, struct sockaddr *addr, socklen_t *addrlen)
{
	struct epoll_event	ev, events[MAX_EVENTS];
	int					conn_sock, nfds, epollfd;

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, TIME_OUT);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
				conn_sock = accept(listen_sock, (struct sockaddr *) addr, addrlen);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
							&ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				req_handler(events[n].data.fd);
			}
		}
	}
}

int	create_server_socket(struct sockaddr_in *servaddr, int addrlen)
{
	int listenfd;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		perror("socket");
		return (-1);
	}
	bzero(servaddr, addrlen);
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr->sin_port = htons(SERVER_PORT);
	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind");
		return (-1);
	}
	if (listen(listenfd, 10) < 0)
	{
		perror("listen");
		return (-1);
	}
	return (listenfd)
}

int	main( void )
{
	socklen_t			addrlen;
	int					listenfd;
	struct sockaddr_in	servaddr;

	addrlen = sizeof(servaddr);
	create_server_socket(&servaddr, addrlen);
	epoll_handler(listenfd, (struct sockaddr *) &servaddr, &addrlen);
	close(listenfd);
	return (0);
}
