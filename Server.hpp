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

        ConfigServer		_configServer;

        std::string			_server_name;
        int                 _serv_port;
        sockaddr_in         _addr;
        socklen_t           _addrlen;
        int                 _serv_socket;

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
        sockaddr*           getAddr() const;
        socklen_t           getAddrlen() const;
        int                 getSock() const;
        int                 getPorts() const;
		ConfigServer&       getConfig( void );

        /* Methods */
		int			create_server_socket( void );
		int			setnonblocking( int );
		int			handle_read( int );
};

#endif
