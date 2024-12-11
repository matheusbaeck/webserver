/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:09:53 by glacroix          #+#    #+#             */
/*   Updated: 2024/12/11 15:02:34 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"
#include "Selector.hpp"

//TODO: if pathConfigFile doesn't exist; exception
//TODO: if parser error EXIT()
ServerManager::ServerManager(const char *pathConfigFile) : _configFile(pathConfigFile) 
{
    int nb_servers = (int)_configFile.getServersConfig().size();
    for (int i = 0; i < nb_servers; i += 1)
    {
        try 
        {
            Server* server = new Server(_configFile.getServersConfig()[i]);
            _servers.push_back(server);
        } 
        catch (std::runtime_error& e) 
        {
            std::cerr << "Caught a runtime error: " << e.what() << std::endl; 
        }
    }
    if (_servers.empty())
        throw std::runtime_error("No servers were successfully initialized. Exiting...");
}

const std::vector<Server *>& ServerManager::getServers() const 
{
    return (_servers);
}

void ServerManager::addSockets(Selector& selector)
{
    for (size_t i = 0; i < this->getServers().size(); i += 1)
    {
        selector.addSocket(this->getServers()[i]);
    }
}

ServerManager::~ServerManager() 
{
    for (size_t i = 0; i < this->getServers().size(); i += 1)
        delete _servers[i];
}
