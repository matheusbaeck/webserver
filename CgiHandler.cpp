/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/10 19:39:38 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

CgiHandler::CgiHandler(HttpRequest _httpReq, std::string cgiPath, std::string scriptName)
{
    if (0)
    {
    this->pipeFd[STDIN_FILENO]  = -1;
    this->pipeFd[STDOUT_FILENO] = -1;


    this->httpReq = _httpReq;


    //this->env["AUTH_TYPE"] = ;
    this->env["CONTENT_LENGTH"]  = httpReq.getHeader("content-length");
    this->env["CONTENT_TYPE"]    = httpReq.getHeader("content-type");
    this->env["QUERY_STRING"]    = httpReq.getQuery();
    this->env["REQUEST_METHOD"]  = httpReq.getMethodStr();
    this->env["SERVER_PORT"]     = httpReq.getServerPort();

    this->env["PATH_INFO"]       = cgiPath; 
    this->env["SCRIPT_FILENAME"] = cgiPath + scriptName;
    
    // TODO: we are just hardcoding SERVER_NAME. Do we need to match it with server_name in config?
    this->env["SERVER_NAME"]       = "localhost";
    this->env["SERVER_PROTOCOL"]   = "HTTP/1.1";
    this->env["SERVER_SOFTWARE"]   = "webserv/0.42";
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->env["REMOTE_HOST"]       = "localhost";
    this->env["REMOTE_ADDR"] = "127.0.0.1";
    std::cout << "HERE\n";
    }
}

std::string CgiHandler::execute(void)
{
    int pid;
    int exitStatus;
    std::string response;

    if (pipe(this->pipeFd) == -1)
    {
        return this->httpReq.serverError();
    }

    pid = fork();

    if (pid == -1)
    {
        return this->httpReq.serverError();
    }

    if (pid == 0)
    {
        std::cerr << "Child process\n";
        
        close(this->pipeFd[STDIN_FILENO]);
        dup2(this->pipeFd[STDOUT_FILENO], STDOUT_FILENO);



        const char *path = "./myCGI";//"/home/glacroix/Documents/testing_webserver/cgi";
        char *envp[this->env.size() + 1];

        std::map<std::string, std::string>::iterator it = this->env.begin();
        for (size_t i = 0; i < this->env.size() && it != this->env.end(); i += 1)
        {
            std::string var = it->first + "=" + it->second;
            envp[i] = (char *)var.c_str();
            it++;
        }
        envp[this->env.size()] = NULL;

        char *const argv[] = { (char *)"./myCGI", NULL};
        
        if (execve(path, argv,  envp) != 0)
        {
            return this->httpReq.serverError();
        }
    }
    else {
    
        wait(&exitStatus);
        char buffer[1024];
        
        close(this->pipeFd[STDOUT_FILENO]);
        dup2(this->pipeFd[STDIN_FILENO], STDIN_FILENO);

        
        int n = 1;
        while (n != 0)
        {
            n = read(STDIN_FILENO, buffer, sizeof(buffer));
            buffer[n] = 0;
            response += buffer;
        }
    }
    return response;
}

CgiHandler::~CgiHandler(void)
{
    close(this->pipeFd[STDIN_FILENO]);
    close(this->pipeFd[STDOUT_FILENO]);
}

        /*SCRIPT_FILENAME $script_path;      # Full path to the script DONE*/
        /*QUERY_STRING $query_string;        # URL query string DONE*/
        /*REQUEST_METHOD $request_method;    # HTTP request method DONE*/
        /*CONTENT_TYPE $content_type;        # Content-Type header*/
        /*CONTENT_LENGTH $content_length;    # Content-Length header*/
        /*SERVER_PROTOCOL $server_protocol;  # Protocol and version*/
        /*SERVER_NAME $server_name;          # Server name*/
        /*PATH_INFO $path_info;              # Extra path information*/

        /*REMOTE_ADDR $client_ip;            # Client's IP address*/

// ssh IDENT@[ADRR or HOST]
