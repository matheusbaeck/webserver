/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/09 20:20:19 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/13 21:07:06 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <cstring>

/*static void mySleep(double sec)*/
/*{*/
/*        struct timeval tv;*/
/*        tv.tv_sec = 0;      // 0 seconds*/
/*        tv.tv_usec = 1000000 * sec; // 100,000 microseconds (0.1 seconds)*/
/**/
/*        select(0, NULL, NULL, NULL, &tv); */
/*}*/


CgiHandler::CgiHandler(HttpRequest* httpReq, std::string cgiPath, std::string scriptName)
{
    this->pipeFd[STDIN_FILENO]  = -1;
    this->pipeFd[STDOUT_FILENO] = -1;
    // TODO: do deep copy of config file to prevent double free
    //this->httpReq = _httpReq;


    //this->env["AUTH_TYPE"] = ;
    this->env["CONTENT_LENGTH"]         = httpReq->getHeader("content-length");
    this->env["CONTENT_TYPE"]           = httpReq->getHeader("content-type");
    this->env["QUERY_STRING"]           = httpReq->getQuery();
    this->env["REQUEST_METHOD"]         = httpReq->getMethodStr();
    this->env["SERVER_PORT"]            = httpReq->getServerPort();

    this->env["PATH_INFO"]              = cgiPath; 
    // TODO: if there is another slash
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
    clock_t end;

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

    this->duration = clock();
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
            std::cout << "elapsed time: " << static_cast<double>(end - this->duration) << std::endl;
            end = clock();
            if (static_cast<double>(end - this->duration)/CLOCKS_PER_SEC > CLIENT_TIMEOUT)
            {
                std::cout << "child killed " << std::endl;

                kill(httpReq.getCgiPID(), SIGKILL); 
            }
            n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            buffer[n] = '\0';
            response += buffer;
            std::cerr << "n: " << n << std::endl;
        }
        /*while (n != 0)*/
        /*{*/
        /*    n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);*/
        /*    buffer[n] = '\0';*/
        /*    response += buffer;*/
        /*    std::cerr << "n: " << n << std::endl;*/
        /*}*/
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
