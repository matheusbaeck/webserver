/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:28:21 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/13 21:07:02 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "HttpRequest.hpp"

#include <map>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <ctime>

#define TIMEOUT 200
#define CLIENT_TIMEOUT 2			//client_timeout max time

class CgiHandler
{
    private:
        HttpRequest httpReq;
        int pipeFd[2];
        std::map<std::string, std::string> env;
        clock_t duration;
    public:
        char **getEnvp(void);
        std::string execute();
        CgiHandler(HttpRequest *httpReq, std::string cgiPath, std::string scriptName);
        ~CgiHandler(void);
};

#endif
