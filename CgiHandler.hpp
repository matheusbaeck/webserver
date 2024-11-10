/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:28:21 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/10 18:52:28 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <map>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

#include "HttpRequest.hpp"

class CgiHandler
{
    private:
        HttpRequest httpReq;
        int pipeFd[2];
        std::map<std::string, std::string> env;
    public:
        std::string execute(void);
        CgiHandler(HttpRequest httpReq, std::string cgiPath, std::string scriptName);
        ~CgiHandler(void);
};

#endif
