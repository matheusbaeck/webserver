#ifndef SERVER_HPP__
# define SERVER_HPP__

#include <vector>
#include <iostream>
#include <cstdarg>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include "ConfigFile.hpp"
#include <fcntl.h>
#include <unistd.h>

#define BACKLOG 10
#define BUFFERSIZE 4096

class Server 
{
	private:

        ConfigServer		m_configServer;

        std::string			m_server_name;
        int                 m_serv_port;
        int                 m_id;
        sockaddr_in         m_addr;
        socklen_t           m_addrlen;
        int                 m_serv_socket;

	public:

        Server( const int port );
        /*Old Constructors*/
		Server( std::vector<uint16_t> );
		Server( ConfigServer & );
		Server( const Server & );
		~Server( void );

		/* Operators */
		Server& operator=(const Server& );

        
		/* New Getters */
        sockaddr            *addr() const;
        socklen_t           addrlen() const;
        int                 id() const;
        int                 sock() const;
        int                 port() const;
		ConfigServer		&getConfig( void );

        /* Methods */
		int			create_server_socket( void );
		int			setnonblocking( int );
		int			handle_read( int );
};

std::ostream &operator<<( std::ostream &, const Server & );

#endif
