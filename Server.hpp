#ifndef SERVER_HPP__
# define SERVER_HPP__

#include "ConfigFile.hpp"
#include <vector>
#include <iostream>
#include <cstdarg>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#define BACKLOG 10
#define BUFFERSIZE 4096

class Selector;

class cgiProcessInfo;

class Server 
{
	private:

        ConfigServer		                _configServer;

        std::string			                _server_name;
        std::vector<uint16_t>               _serv_ports;
        std::vector<sockaddr_in>            _addrs;
        std::vector<socklen_t>              _addrlens;
        std::vector<int>                    _serv_sockets;

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
        sockaddr*               getAddr(int pos) const;
        socklen_t               getAddrlen(int pos) const;
        std::vector<int>        getSockets() const;
        std::vector<uint16_t>   getPorts() const;
		ConfigServer&           getConfig( void );

        /* Methods */
		int			create_server_socket(int pos);
		int			setnonblocking( int );

        int         acceptConnection(Selector& selector, int socketFD, int portFD);
        void        readClientRequest(Selector& selector, int clientFD);
        int         handleResponsePipe(Selector& selector, int pipeFd);
		int			sendResponse(Selector& selector, int clientSocket, std::string request);
        void        sendCGIResponse(cgiProcessInfo* cgiInfo);
};

#endif
