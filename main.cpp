#include "ServerManager.hpp"
#include "Selector.hpp"
#include "ConfigFile.hpp"

int main(int argc, char **argv)
{
    try 
    {
        signal(SIGPIPE, SIG_IGN);
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
    catch (std::runtime_error& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;  // Exit if no servers were successfully initialized
    }
    return 0;
}

