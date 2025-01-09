#include "HttpRequest.hpp"
#include "Selector.hpp"
#include <cstring>
#include <map>

Selector Selector::selector;

//TODO: replace all logs with std::cerr or std::cout
//TODO: what happens if epollfd fails
Selector::Selector(void) 
{
	this->_epollfd = epoll_create(1);
	if (this->_epollfd == -1)
    {
        std::cerr << "Failed to create epoll_instance: " << strerror(errno) << std::endl;
	}
}

Selector::~Selector( void )
{
	if (this->_epollfd != -1)
		close(this->_epollfd);
}

epoll_event* Selector::getEvents()
{
    return (this->_events);
}

std::set<int>& Selector::getActiveClients()
{
    return this->_activeClients;
}

std::map<int, cgiProcessInfo*>& Selector::getCgis()
{
    return _cgiProcesses;
}

void Selector::addCgi(int pipeFd, cgiProcessInfo* CgiProcess)
{
    std::cout << "a Cgi process has been added from fd: " << CgiProcess->_clientFd << std::endl;
    _cgiProcesses[pipeFd] = CgiProcess;
}

int Selector::checkCgiStatus(cgiProcessInfo* cgi)
{
    int status = 0;
    waitpid(cgi->_pid, &status, 0);
    return WEXITSTATUS(status);
}

void Selector::deleteCgi(cgiProcessInfo* CgiProcess)
{
    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_DEL, CgiProcess->_responsePipe, NULL) == -1) 
    {
        perror("epoll_ctl: remove eventFd");
        std::cerr << "Failed to remove eventFd: " << CgiProcess->_responsePipe << ", errno: " << errno << std::endl;
    }

     _cgiProcesses.erase(CgiProcess->_responsePipe);
    close(CgiProcess->_responsePipe);
    close(CgiProcess->_pipe[0]);
    close(CgiProcess->_pipe[1]);
    std::cout << "Closed response pipe: " << CgiProcess->_responsePipe << std::endl;

    CgiProcess->_path.clear();
    CgiProcess->_ScriptResponse.clear();
    delete CgiProcess;
    std::cout << "Deleted CGI Process." << std::endl;
}

std::map<int, ConfigServer>& Selector::getClientConfig()
{
    return (this->_clientConfig);
}

std::map<int, std::string>& Selector::getRequests()
{
    return (this->_requests);
}

std::map<int, HttpRequest*>& Selector::getHTTPRequests()
{
    return (this->_httpRequests);
}

int& Selector::getEpollFD()
{
    return (this->_epollfd);
}


void Selector::addSocket(const Server *server)
{
    for (size_t i = 0; i < server->getSockets().size(); i += 1)
    {
        int socket_fd = server->getSockets()[i];
        _ev.events = EPOLLIN | EPOLLET;
        _ev.data.fd = socket_fd;

        if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, socket_fd, &_ev) == -1) 
        {
            std::cerr << "addSocket: Failed to add socket fd " << socket_fd << " to epoll: " << strerror(errno) << std::endl;
            return;
        }

    }
}

bool Selector::isServerSocket(int fd, int serverSocket)
{

    return (fd == serverSocket); 
}

bool Selector::isClientFD(int fd)
{
    return (this->_activeClients.find(fd) != _activeClients.end());
}

bool Selector::isResponsePipe(int event_fd) const
{
    if (_cgiProcesses.size())
    {
        std::map<int, cgiProcessInfo*>::const_iterator it = _cgiProcesses.begin();
        while (it != _cgiProcesses.end())
        {
            if (event_fd == it->second->_responsePipe)
                return true;
            it++;
        }
    }
    return (false);
}

bool Selector::isRequestChunked(int clientFd)
{
    std::string request = selector.getRequests()[clientFd];
    size_t found = request.find("Transfer-Encoding: chunked");
    return (found != std::string::npos);
}

bool Selector::isHeadersEnd(int clientFd)
{
    static const std::string    RequestHeaderEnding = "\r\n\r\n";

    std::string request = selector.getRequests()[clientFd];
    size_t pos = selector.getRequests()[clientFd].find(RequestHeaderEnding);
    return (pos != std::string::npos); 
}

size_t Selector::getBodyContentLength(int clientFd)
{
    std::string contentLenStr = "Content-Length: "; 
    std::string request = selector.getRequests()[clientFd];
    size_t pos = request.find(contentLenStr);
    if (pos != std::string::npos)
    {
        std::stringstream ss(request.substr(pos + contentLenStr.size()));
        size_t num;
        ss >> num;
        return (num);
    }
    return (pos);

}

void Selector::setClientFdEvent(int event_fd, int action)
{
    struct epoll_event info; 
    if (action == READ)
        info.events = EPOLLIN | EPOLLET; 
    else
        info.events = EPOLLOUT | EPOLLET; 

    info.data.fd = event_fd;
    if (epoll_ctl(selector.getEpollFD(), EPOLL_CTL_MOD, event_fd, &info) == -1) 
    {
        std::cerr << "Failed to modify epoll event for FD " << event_fd 
            << ": " << strerror(errno) << std::endl;
        close(event_fd);
    }
}

void Selector::removeClient(int clientSocket)
{
    if (epoll_ctl(getEpollFD(), EPOLL_CTL_DEL, clientSocket, NULL) == -1)
    {
        perror("epoll_ctl: remove eventFd");
        std::cerr << "Failed to remove eventFd: " << clientSocket << ", errno: " << errno << std::endl;
        return;
    }
    getClientConfig().erase(clientSocket);
    getHTTPRequests().erase(clientSocket);
    getActiveClients().erase(clientSocket);
    getRequests().erase(clientSocket);
    close(clientSocket);
    std::cout << "Removed clientSocket: " << clientSocket << std::endl;
}

void Selector::examineCgiExecution()
{
    std::map<int, cgiProcessInfo*>::const_iterator it = _cgiProcesses.begin();
    int status;

    while (it != _cgiProcesses.end())
    {
        int result = waitpid(it->second->_pid, &status, WNOHANG);
        if (!result && std::time(0) -  it->second->getStartTime() > CLIENT_TIMEOUT)
            break;
        if (WIFEXITED(status))
        {
            std::string response = HttpRequest::serverError("");
            send(it->second->_clientFd, response.c_str(), response.size(), 0);
            removeClient(it->second->_clientFd);        
            deleteCgi(it->second);
            return;
        }
        it++;
    }
    if (it != _cgiProcesses.end())
    {
        std::cout << "we killed someone" << std::endl;
        kill(it->second->_pid, SIGKILL);

        std::string response = HttpRequest::gatewayTimeout("");
        send(it->second->_clientFd, response.c_str(), response.size(), 0);

        removeClient(it->second->_clientFd);        
        deleteCgi(it->second);
    }
}

void Selector::processEvents(const std::vector<Server*>& servers )
{
    if (_cgiProcesses.size() > 0)
        examineCgiExecution();
    _eventCount = epoll_wait(_epollfd, _events, MAX_EVENTS, TIME_OUT); 
    if (_eventCount == 0)
        return;
    if (_eventCount == -1) 
    {
        std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        return;
    }
    int err;
    for (int n = 0; n < _eventCount; ++n) 
    {
        for (size_t i = 0; i < servers.size(); i += 1)
        {
            Server *server = servers[i];
            int event_fd = _events[n].data.fd;

            if (_events[n].events & (EPOLLERR | EPOLLHUP)) 
            {
                std::cerr << "Error on FD " << _events[n].data.fd << ": " << strerror(errno) << std::endl;
                removeClient(event_fd);
            }
            // Reading requests
            if (_events[n].events & EPOLLIN) 
            {

                for (size_t j = 0; j < server->getSockets().size(); ++j) 
                {
                    int serverSocket    = server->getSockets()[j];
                    int serverPort      = server->getPorts()[j];
                    if (isServerSocket(event_fd, serverSocket)) 
                    {
                        std::cout << "event_Fd: " << event_fd << std::endl;
                        err = server->acceptConnection(*this, serverSocket, serverPort); // Accept new client connection
                        if (err == -1)
                            continue;
                    }
                    else if (isClientFD(event_fd))
                    {
                        std::cout << "----------------------isClientFd" << std::endl;
                        server->readClientRequest(*this, event_fd); // Read client request
                        return;
                    }
                    else if (isResponsePipe(event_fd))
                    {
                        std::cout << "----------------------isResponsePipe" << std::endl;
                        int err = server->handleResponsePipe(*this, event_fd); // Handle CGI output
                        if (err == -1)
                            return;
                    }
                }
            }
            // Answering clients
            if (_events[n].events & EPOLLOUT) 
            {
                server->sendResponse(*this, event_fd);
                selector.getRequests().erase(event_fd);
                return;
            }
        }
    }
}
