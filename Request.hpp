#ifndef REQUEST_HPP__
# define REQUEST_HPP__

#include <iostream>

class Request
{
	private:
		int	m_fd;
		int m_port;
	public:

		Request( int );
		Request( int, int );
		~Request( void );

		int	fd( void );
		int	port( void );

		void handler(void);
};

std::ostream	&operator<<( std::ostream &, const Request & );

#endif