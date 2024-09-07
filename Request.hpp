#ifndef REQUEST_HPP__
# define REQUEST_HPP__

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <stdio.h>


#define BUFFERSIZE 4096

class Request
{
	private:
		int	m_fd;
		int m_port;
	public:

		Request( int );
		Request( int, int );
		~Request( void );

		int	fd( void ) const;
		int	port( void ) const;

		void handler(void);
};

std::ostream	&operator<<( std::ostream &, const Request & );

#endif