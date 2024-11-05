#include "ServerManager.hpp"
#include "Selector.hpp"
#include "ConfigFile.hpp"

#if 1

int main(int argc, char **argv)
{
    if (argc > 2)
	{
        std::cerr << "too many arguments" << std::endl;
		return 1;
	}

    const char *pathname;
    if (argc == 2)
        pathname = argv[1];
    else
        pathname = ConfigFile::pathname;
    
    ServerManager manager(pathname);
    Selector& selector = Selector::getSelector();
    manager.addSockets(selector);
    std::cout << "--------------------------------------------------> Starting infinite loop\n" << std::endl;
    for ( ; ; )
        selector.processEvents(manager.getServers());
}

#else 


int main(int argc, char **argv)
{
    if (argc > 2)
	{
        std::cerr << "too many arguments" << std::endl;
		return 1;
	}

    const char *pathname;
    if (argc == 2)
        pathname = argv[1];
    else
        pathname = ConfigFile::pathname;
    
    ServerManager manager(pathname);
    Selector& selector = Selector::getSelector();
    exit(1);
    manager.addSockets(selector);
    std::cout << "--------------------------------------------------> Starting infinite loop\n" << std::endl;
}


#endif
