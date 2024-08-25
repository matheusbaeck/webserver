
#ifndef SERVER_HPP__
# define SERVER_HPP__

#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdarg>

#include "Worker.hpp"

class Server
{
	private:
		std::vector<Worker>	m_workers;
		std::string			m_server_name;
		/* all other att of the server */

	public:
		Server( void );
		Server( std::vector<int> );
		Server( int , ... );
		~Server();

		void addWorker(const Worker& worker);
};


#endif
