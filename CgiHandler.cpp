/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/12/15 14:35:55 by glacroix         ###   ########.fr       */
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

cgiProcessInfo::~cgiProcessInfo() 
{
    /*close(_pipe[0]);*/
    /*close(_pipe[1]);*/
}

CgiHandler::CgiHandler(HttpRequest *_httpReq, std::string scriptName, std::string cgiPath)
{
    // TODO: do deep copy of config file to prevent double free
    this->httpReq = _httpReq;
    //get request
    //find end of headers
    //do substr from the end of headers + sizeof \r\n\r\n
    //use that to write within pipe as input

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
       /*if (envp[i] != NULL) return NULL;*/
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
    select(0, NULL, NULL, NULL, &timeout);
}

static void freeDoublePointer(char **envp)
{
    for (size_t i = 0; envp[i]; i +=1)
        free(envp[i]);
    delete[] envp;
}

StatusCode CgiHandler::execute(Selector& selector, int clientFd, int bodyPipe)
{
    cgiProcessInfo* cgiInfo = new cgiProcessInfo();

    // Assign to CgiHandlerInfo
    //char **envp = this->getEnvp();
    char **envp;
    if (pipe(cgiInfo->_pipe) == -1)
    {
        perror("pipe");
        delete cgiInfo;
        return SERVERR;
    }
    const char *path = this->env["SCRIPT_FILENAME"].c_str();
    if (access(path, X_OK) == -1) 
    { 
        perror("access");
        delete cgiInfo;
        if (errno == EACCES) return FORBIDDEN; // Forbidden
        if (errno == ENOENT || errno == ENOTDIR) return NFOUND; // Not Found
        if (errno == ELOOP || errno == ENOMEM || errno == EFAULT) return SERVERR; // Internal Server Error
        if (errno == ENAMETOOLONG) return BREQUEST; // Bad Request
    }
    envp = this->getEnvp();
    cgiInfo->_pid = fork();
    if (cgiInfo->_pid == -1) 
    {
        perror("fork");
        close(cgiInfo->_pipe[0]);
        close(cgiInfo->_pipe[1]);
        delete cgiInfo;
        freeDoublePointer(envp);
        return SERVERR;
    }
    
    if (cgiInfo->_pid == 0)
    {

        dup2(bodyPipe, STDIN_FILENO); // CGI reads from inputPipe
        dup2(cgiInfo->_pipe[1], STDOUT_FILENO); // CGI writes to pipe
        dup2(cgiInfo->_pipe[1], STDERR_FILENO); // CGI error writes to pipe
        close(cgiInfo->_pipe[0]);
        close(cgiInfo->_pipe[1]);
        close(bodyPipe);
        
        char *const argv[] = { (char*)path, NULL};

        if (execve(argv[0], argv, envp) == -1) 
        {
            freeDoublePointer(envp);
            delete cgiInfo;
            perror("execve");
            std::cerr << "errno: " << errno << std::endl;
            if (errno == EACCES) exit(3); // Forbidden
            if (errno == ENOENT || errno == ENOTDIR) exit(1); // Not Found
            if (errno == E2BIG) exit(5); // Bad Request
            if (errno == ELOOP || errno == ENOMEM || errno == EFAULT || errno == ETXTBSY || errno == EINVAL) exit(2); // Internal Server Error
            if (errno == EPERM || errno == 13) exit(4); // Forbidden
            exit(errno);
        }
    }
    else
    {
        close(cgiInfo->_pipe[1]);
        waitMicroseconds(2000);
        int status;
        int res = waitpid(cgiInfo->_pid, &status, WNOHANG);
        std::cout << "res of waitpid: " << res << std::endl;
        if (WIFEXITED(status))
        {
            int err = WEXITSTATUS(status);
            std::cout << "status is " << err << std::endl;
            if (err != 0)
            {
                freeDoublePointer(envp);
                delete cgiInfo;
                if (err == 1) return NFOUND;
                if (err == 2) return SERVERR;
                if (err == 3) return FORBIDDEN;
                if (err == 4) return FORBIDDEN;
                if (err == 5) return BREQUEST;
                return FORBIDDEN;
            }
        }
        
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = cgiInfo->_pipe[0];
        std::cout << "cgiInfo.responsePipe: " << cgiInfo->_pipe[0] << std::endl;
        if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_ADD, cgiInfo->_pipe[0], &ev) == -1) 
        {
            perror("epoll_ctl: cgiInfo->_pipe[0]");
            close(cgiInfo->_pipe[0]);
            freeDoublePointer(envp);
            delete cgiInfo;
            return SERVERR;
        }
        std::cout << "added ResponsePipe to epoll instance: " << cgiInfo->_pipe[0] << std::endl;

        cgiInfo->addProcessInfo(cgiInfo->_pid, clientFd, cgiInfo->_pipe[0], this->getEnvMap()["SCRIPT_FILENAME"]);
        selector.addCgi(cgiInfo->_pipe[0], cgiInfo);
    }
    freeDoublePointer(envp);
    return OK;
}

CgiHandler::~CgiHandler(void) {}

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
