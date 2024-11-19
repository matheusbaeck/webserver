/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/19 20:40:58 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <cstring>

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>


CgiHandler::CgiHandler(HttpRequest _httpReq, std::string scriptName, std::string cgiPath)
{
    this->pipeFd[STDIN_FILENO]  = -1;
    this->pipeFd[STDOUT_FILENO] = -1;

    // TODO: do deep copy of config file to prevent double free
    this->httpReq = _httpReq;


    //this->env["AUTH_TYPE"] = ;
    this->env["CONTENT_LENGTH"]         = _httpReq.getHeader("content-length");
    this->env["CONTENT_TYPE"]           = _httpReq.getHeader("content-type");
    this->env["QUERY_STRING"]           = _httpReq.getQuery();
    this->env["REQUEST_METHOD"]         = _httpReq.getMethodStr();
    this->env["SERVER_PORT"]            = _httpReq.getServerPort();

    /*// TODO: if there is another slash*/
    this->env["PATH_INFO"]              = cgiPath;
    this->env["SCRIPT_FILENAME"]        = cgiPath + "/" + scriptName;
    
    // TODO: we are just hardcoding SERVER_NAME. Do we need to match it with server_name in config?
    this->env["SERVER_NAME"]            = "localhost";
    this->env["SERVER_PROTOCOL"]        = "HTTP/1.1";
    this->env["SERVER_SOFTWARE"]        = "webserv/0.42";
    this->env["GATEWAY_INTERFACE"]      = "CGI/1.1";
    this->env["REMOTE_HOST"]            = "localhost";
    this->env["REMOTE_ADDR"]            = "127.0.0.1";
}

char    **CgiHandler::getEnvp(void)
{
   char **envp = new char*[this->env.size() + 1];
   std::map<std::string, std::string>::iterator it = this->env.begin();

   for (size_t i = 0; i < this->env.size() && it != this->env.end(); i += 1)
   {
       std::string var = it->first + "=" + it->second;
       envp[i] = strdup(var.c_str());
       it++;
   }
   envp[this->env.size()] = NULL;
   return envp;
}

std::string CgiHandler::execute()
{
    std::string response;
    int exitStatus;

    char **envp = this->getEnvp();

    if (pipe(this->pipeFd) == -1)
    {
        std::cout << "pipe dead\n";
        return httpReq.serverError();
    }

    httpReq.setCgiPID(fork());

    if (httpReq.getCgiPID() == -1)
    {
        std::cout << "fork is spoon now\n";
        return httpReq.serverError();
    }

    if (httpReq.getCgiPID() == 0)
    {
        close(this->pipeFd[STDIN_FILENO]);
        dup2(this->pipeFd[STDOUT_FILENO], STDOUT_FILENO);
        close(this->pipeFd[STDOUT_FILENO]);
        
        std::cerr << "Child process\n";


        const char *path = this->env["SCRIPT_FILENAME"].c_str();

        std::cerr << "path: " << path << std::endl;

        char *const argv[] = { (char*)path, NULL};
        if (access(path, R_OK | W_OK | X_OK | F_OK) == -1)
        {
            if (errno == 2)
                return this->httpReq.notFound();
            else
                return this->httpReq.forbidden();

        }
        if (execve(argv[0], argv,  envp) != 0)
        {
            std::cerr << "execute is soup\n";
            close(this->pipeFd[STDOUT_FILENO]);
            return this->httpReq.serverError();
        }
    }
    else
    {
        close(this->pipeFd[STDOUT_FILENO]);
        dup2(this->pipeFd[STDIN_FILENO], STDIN_FILENO);
        close(this->pipeFd[STDIN_FILENO]);
        waitpid(httpReq.getCgiPID(), &exitStatus, WNOHANG);
        
       // mySleep(0.1);
        char buffer[1024];
        int n = 1;
        while (n != 0)
        {
            n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            buffer[n] = '\0';
            response += buffer;
        }
    }

    delete[] envp;
    return response;
}

CgiHandler::~CgiHandler(void)
{
    //close(this->pipeFd[STDIN_FILENO]);
    //close(this->pipeFd[STDOUT_FILENO]);
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
