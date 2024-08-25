#ifndef SERVERMANAGER_HPP__
# define SERVERMANAGER_HPP__

#include <vector>

#include "Server.hpp"


class Server;

class ServerManager
{
	private:
		std::vector<Server> servers;

	public:
		ServerManager( void );
		ServerManager( int, ... );
		ServerManager( std::vector<std::vector<int>> );
		~ServerManager( void );

		void addServer(const Server& server);
};

#endif