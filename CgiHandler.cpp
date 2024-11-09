/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/09 20:31:22 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

CgiHandler::CgiHandler(HttpRequest httpReq)
{
    this->env["AUTH_TYPE"] = ;
    this->env["CONTENT_LENGTH"] = ;
    this->env["CONTENT_TYPE"] = ;
    this->env["GATEWAY_INTERFACE"] = ;
    this->env["PATH_INFO"] = ;
    this->env["PATH_TRANSLATED"] = ;
    this->env["QUERY_STRING"] = ;
    this->env["REMOTE_ADDR"] = ;
    this->env["REMOTE_HOST"] = ;
    this->env["REMOTE_IDENT"] = ;
    this->env["REMOTE_USER"] = ;
    this->env["REQUEST_METHOD"] = httpReq.getMethod();
    this->env["SCRIPT_NAME"] = ;
    this->env["SERVER_NAME"] = ;
    this->env["SERVER_PORT"] = ;
    this->env["SERVER_PROTOCOL"] = ;
    this->env["SERVER_SOFTWARE"] = ;
}
