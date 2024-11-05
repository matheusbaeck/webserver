/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:01:57 by glacroix          #+#    #+#             */
/*   Updated: 2024/10/30 10:54:07 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include "ConfigFile.hpp"
#include "Server.hpp"
#include <vector>

class Selector;

class ServerManager
{
    private:
        ConfigFile _configFile;
        int _nb_servers;
        std::vector<Server*> _servers;
    public:
        const std::vector<Server*> getServers() const;
        int getSize() const;
        void addSockets(Selector &selector);

        ServerManager(const char *pathConfigFile);
        ~ServerManager();
};

#endif
