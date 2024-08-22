/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: math <math@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 13:37:09 by math              #+#    #+#             */
/*   Updated: 2024/08/20 20:28:57 by math             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> //memset, bzero
#include <arpa/inet.h> //inet_pton [convert sting ip to binary]


#define SERVER_PORT	80
#define MAXLINE		4096


int	main( int argc, char **argv )
{
	int					sockfd, n;
	int					sendbytes;
	struct sockaddr_in	servaddr;
	char				sendline[MAXLINE];
	char				recvline[MAXLINE];

	if (argc != 2)
	{
		printf("Usage: %s <IP address>\n", argv[0]);
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return (perror("socket"), 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		return (perror("inet_pton"), 0);

	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		return (perror("connect"), 0);

	memset(sendline, 0, MAXLINE);
	sprintf(sendline, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", argv[1]);
	sendbytes = strlen(sendline);
	if (write(sockfd, sendline, sendbytes) != sendbytes)
		return (printf("error\n"), perror("write"), 1);
	printf("hey\n");
	memset(recvline, 0, MAXLINE);
	while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
	{
		recvline[n] = '\0';
		printf("%s", recvline);
	}
	if (n < 0)
		return(perror("read"), 0);
	return (0);
}