
#ifndef SERVER_HPP__
# define SERVER_HPP__

#include <vector>
#include "Worker.hpp"

class Server
{
	private:
		std::vector<Worker>	m_workers;
		std::string			m_server_name;
		/* all other att of the server */

	public:
		Server(/* args */);
		~Server();

		void addWorker(const Worker& worker);
};


#endif
