/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/12/03 18:47:37 by glacroix         ###   ########.fr       */
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

cgiProcessInfo::cgiProcessInfo() {}

std::time_t& cgiProcessInfo::getStartTime()
{
    return (this->_startTime);
}

void cgiProcessInfo::addProcessInfo(int pid, int clientFd, int responsePipe, std::string scriptFileName) 
{
    _pid = pid;
    _clientFd = clientFd;
    _responsePipe = responsePipe;
    _path = scriptFileName;
    _startTime = std::time(0);
}

cgiProcessInfo::~cgiProcessInfo() {}

CgiHandler::CgiHandler(HttpRequest *_httpReq, std::string scriptName, std::string cgiPath)
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

std::map<std::string, std::string>& CgiHandler::getEnvMap()
{
    return (this->env);
}

void waitMicroseconds(long microseconds) 
{
    struct timeval timeout;
    timeout.tv_sec = microseconds / 1000000;       // Convert to seconds
    timeout.tv_usec = microseconds % 1000000;     // Remaining microseconds

    // Call select with no file descriptors
    select(0, NULL, NULL, NULL, &timeout);
}

StatusCode CgiHandler::execute(Selector& selector, int clientFd)
{

    cgiProcessInfo* cgiInfo = new cgiProcessInfo();

    // Assign to CgiHandlerInfo
    char **envp = this->getEnvp();
    if (pipe(cgiInfo->_pipe) == -1)
    {
        perror("pipe");
        return SERVERR;
    }

    const char *path = this->env["SCRIPT_FILENAME"].c_str();
    if (access(path, X_OK) == -1) 
    { 
        perror("access");
        std::cerr << "error is " << errno << std::endl; 
        if (errno == 13)
            return FORBIDDEN;
        return NFOUND; //should just say 404
    }
    int pid = fork();
    if (pid == -1) 
    {
        perror("fork");
        close(cgiInfo->_pipe[0]);
        close(cgiInfo->_pipe[1]);
        delete cgiInfo;
        return SERVERR;
    }
    
    if (pid == 0)
    {

        dup2(cgiInfo->_pipe[1], STDOUT_FILENO); // CGI writes to pipe
        close(cgiInfo->_pipe[0]);
        close(cgiInfo->_pipe[1]);
        
        std::cerr << "path: " << path << std::endl;
        char *const argv[] = { (char*)path, NULL};

        if (execve(argv[0], argv, envp) == -1) 
        {
            perror("execve");
            exit(2);
        }
    }
    else
    {
        close(cgiInfo->_pipe[1]);
        //TODO: problem with removeClient with infinite while if i don't wait i cannot get the other error pages
        waitMicroseconds(1000);
        int status;
        int res = waitpid(cgiInfo->_pid, &status, WNOHANG);
        std::cout << "result of waitpid is: " << res << std::endl;
        if (res)
        {
            if (WIFEXITED(status))
            {
                
                int err = WEXITSTATUS(status);
                std::cout << "status is " << err << std::endl;
                if (err == 1 || err == 2 || err == 3)
                {
                    delete[] envp;
                    delete cgiInfo;
                    if (err == 1) return NFOUND;
                    if (err == 2) return SERVERR;
                    if (err == 3) return FORBIDDEN;
                }
            }
        }
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = cgiInfo->_pipe[0];
        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, cgiInfo->_pipe[0], &ev) == -1) 
        {
            perror("epoll_ctl: cgiInfo->_pipe[0]");
            close(cgiInfo->_pipe[0]);
            delete[] envp;
            return SERVERR;
        }
        std::cout << "added ResponsePipe to epoll instance: " << cgiInfo->_pipe[0] << std::endl;

        cgiInfo->addProcessInfo(pid, clientFd, cgiInfo->_pipe[0], this->getEnvMap()["SCRIPT_FILENAME"]);
        selector.addCgi(cgiInfo->_pipe[0], cgiInfo);
        delete[] envp;
    }
    return OK;
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
