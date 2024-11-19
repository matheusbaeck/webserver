/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:28:21 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/19 20:41:15 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "HttpRequest.hpp"

#include <map>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <cstring>

#define TIMEOUT 200
#define CLIENT_TIMEOUT 2			//client_timeout max time

class CgiHandler
{
    private:
        HttpRequest httpReq;
        std::string cgiResponse;
        int pipeFd[2];
        std::map<std::string, std::string> env;
    public:
        char **getEnvp(void);
        std::string execute();
        CgiHandler(HttpRequest httpReq, std::string scriptName, std::string cgiPath);
        ~CgiHandler(void);
};

#endif
