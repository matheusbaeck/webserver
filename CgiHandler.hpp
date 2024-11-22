/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:28:21 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/22 16:42:32 by glacroix         ###   ########.fr       */
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

struct cgiProcessInfo
{
    std::pair<int, int> responsePipe;
    std::pair<int, int> statusPipe;
    int pid;
    int clientFd;
};

class Cgi
{
    private:
        cgiProcessInfo                      info;
        HttpRequest                         *httpReq;
        std::string                         cgiResponse;
        std::map<std::string, std::string>  env;
    public:
        char**                              getEnvp(void);
        StatusCode                          execute(Selector& selector, int clientFd);

        Cgi(HttpRequest *httpReq, std::string scriptName, std::string cgiPath);
        ~Cgi(void);
};

#endif
