/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:09:53 by glacroix          #+#    #+#             */
/*   Updated: 2024/10/30 16:07:39 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"
#include "Selector.hpp"

//TODO: if pathConfigFile doesn't exist; exception
ServerManager::ServerManager(const char *pathConfigFile) : _configFile(pathConfigFile) 
{
    _nb_servers = (int)_configFile.getServersConfig().size();
    for (int i = 0; i < _nb_servers; i += 1)
    {
        Server* server = new Server(_configFile.getServersConfig()[i]);
        _servers.push_back(server);
    }
}

const std::vector<Server *> ServerManager::getServers() const
{
    return (_servers);
}

int ServerManager::getSize() const
{
    return (_nb_servers);
}

void ServerManager::addSockets(Selector& selector)
{
    for (int i = 0; i < this->getSize(); i += 1)
        selector.addSocket(this->getServers()[i]);
}

ServerManager::~ServerManager() 
{
    for (int i = 0; i < _nb_servers; i += 1)
        delete _servers[i];
}
