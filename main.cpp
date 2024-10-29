#include "Server.hpp"
#include "Selector.hpp"
#include "ConfigFile.hpp"
#include <vector>

int main(int argc, char **argv)
{
	
	const char *pathname = ConfigFile::pathname;

	(void) pathname;
	if (argc > 2)
	{
        std::cerr << "too many arguments" << std::endl;
		return 1;
	}
	if (argc == 2)
        pathname = argv[1];

	ConfigFile configFile(pathname);
	Selector& selector = Selector::getSelector();
    size_t nb_servers = (int)configFile.getServers().size();

    std::vector<Server*> servers;
    for (size_t i = 0; i < nb_servers; i += 1)
    {
        Server *server = new Server(configFile.getServer(i));
        selector.addSocket(server);
        servers.push_back(server);
    }

	for ( ; ; )
    {
			selector.processEvents(servers);
	} 
    for (size_t i = 0; i < nb_servers; i += 1)
        delete servers[i];
    return(0); 
}
