/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/22 18:59:57 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include "Selector.hpp"
#include "HttpRequest.hpp"

#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>


Cgi::Cgi(HttpRequest *_httpReq, std::string scriptName, std::string cgiPath)
{
    // TODO: do deep copy of config file to prevent double free
    this->httpReq = _httpReq;


    //this->env["AUTH_TYPE"] = ;
    this->env["CONTENT_LENGTH"]         = _httpReq->getHeader("content-length");
    this->env["CONTENT_TYPE"]           = _httpReq->getHeader("content-type");
    this->env["QUERY_STRING"]           = _httpReq->getQuery();
    this->env["REQUEST_METHOD"]         = _httpReq->getMethodStr();
    this->env["SERVER_PORT"]            = _httpReq->getServerPort();

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

char    **Cgi::getEnvp(void)
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

StatusCode Cgi::execute(Selector& selector, int clientFd)
{
    int responsePipe[2];
    int statusPipe[2];

    // Assign to CgiInfo
    char **envp = this->getEnvp();
    
    this->info.clientFd = clientFd;
    if (pipe(responsePipe) == -1)
    {
        perror("pipe");
        return SERVERR;
    }
    if (pipe(statusPipe) == -1)
    {
        perror("pipe");
        return SERVERR;
    }

    this->info.pid = fork();
    if (this->info.pid == -1) 
    {
        perror("fork");
        close(responsePipe[0]);
        close(responsePipe[1]);
        close(statusPipe[0]);
        close(statusPipe[1]);
        return SERVERR;
    }
    
    if (this->info.pid == 0)
    {
        close(statusPipe[0]);
        dup2(responsePipe[1], STDOUT_FILENO); // CGI writes to pipe
        close(responsePipe[0]);
        close(responsePipe[1]);
        
        const char *path = this->env["SCRIPT_FILENAME"].c_str();
        std::cerr << "path: " << path << std::endl;
        char *const argv[] = { (char*)path, NULL};

        if (access(path, X_OK) == -1) 
        {
            perror("access");
            write(statusPipe[1], "403", 3);
            exit(EXIT_FAILURE);
        }

        if (execve(argv[0], argv, envp) == -1) 
        {
            perror("execve");
            write(statusPipe[1], "500", 3); 
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        //should i just WAITNOHANG here and check for the exit status
        close(statusPipe[1]);
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = responsePipe[0];
        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, responsePipe[0], &ev) == -1) 
        {
            perror("epoll_ctl: responsePipe[0]");
            close(responsePipe[0]);
            close(statusPipe[0]);
            return SERVERR;
        }
        ev.data.fd = statusPipe[0];
        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, statusPipe[0], &ev) == -1)
        {
            perror("epoll_ctl: responsePipe[0]");
            close(responsePipe[0]);
            close(statusPipe[0]);
            return SERVERR;
        }
        close(responsePipe[1]);

        //could turn this into map for multiple cgis
        cgiProcessInfo cgi;
        cgi.responsePipe = std::make_pair(responsePipe[0], -1);
        cgi.statusPipe = std::make_pair(statusPipe[0], -1);
        cgi.clientFd = clientFd;
        cgi.pid = this->info.pid;
        selector.setCgiProcessInfo(cgi);
        delete[] envp;
    }
    return OK;
}

Cgi::~Cgi(void)
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
