/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:28:21 by glacroix          #+#    #+#             */
/*   Updated: 2024/12/02 15:21:35 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "ConfigFile.hpp"

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

class HttpRequest;
class Selector;


class CgiHandler
{
    private:
        HttpRequest                         *httpReq;
        std::string                         cgiResponse;
        std::map<std::string, std::string>  env;
    public:
        char**                              getEnvp(void);
        std::map<std::string, std::string>& getEnvMap(void);
        StatusCode                          execute(Selector& selector, int clientFd);

        CgiHandler(HttpRequest *httpReq, std::string scriptName, std::string cgiPath);
        ~CgiHandler(void);
};

class cgiProcessInfo 
{
    public:
        int                     _responsePipe;
        int                     _pid;
        int                     _clientFd;
        int                     _pipe[2];
        std::string             _path;
        std::string             _ScriptResponse;

        void addProcessInfo(int pid, int clientFd, int responsePipe, std::string scriptFileName); 

        cgiProcessInfo();
        ~cgiProcessInfo();
};

#endif
