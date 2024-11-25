#ifndef SELECTOR_HPP__
# define SELECTOR_HPP__

#include <cstdio>			//perror
#include <unistd.h>			//close
#include <fcntl.h>			//fcntl
#include <sys/epoll.h>		//epoll
#include <sys/types.h>		//accept
#include <sys/socket.h>		//aceppt
#include <set>
#include <ctime>

#include "Server.hpp"
#include "CgiHandler.hpp"

#define MAX_EVENTS 10		//epoll_wait max events at time
#define TIME_OUT 200			//epoll_wait max time
#define READ 0
#define WRITE 1

typedef struct epoll_event epoll_event;

/*struct ClientRequest {*/
/*    int                 clientFd;                        // Client's socket file descriptor*/
/*    HttpRequest*        httpReq;*/
/*    std::string         responseBuffer;          // Response to send back*/
/*    size_t              bytesSent = 0;                // How much of the response has been sent*/
/*    bool                isCGI = false;                  // Whether this request involves CGI*/
/*    std::string         cgiScript;               // Associated CGI script path*/
/*    int                 responsePipeFd = -1;             // CGI response pipe (if applicable)*/
/*    int                 statusPipeFd = -1;               // CGI status pipe (if applicable)*/
/*    time_t              startTime;                    // Request start time*/
/*    time_t              timeout;                      // Request timeout*/
/*};*/


// this class is a singleton (ensures that a class has only one instance and provides a global point of access to that instance)
class Selector 
{
	private:
		static Selector	selector;
		epoll_event                     _ev;
		epoll_event                     _events[MAX_EVENTS];
		int                             _eventCount;
        int                             _epollfd;
        std::set<int>                   _activeClients;
        std::map<int, std::string>      _requests;
        std::map<int, ConfigServer>     _clientConfig;
        std::map<int, cgiProcessInfo>   _cgiProcesses;
		Selector( void );

	public:
		~Selector( void );
		static Selector& getSelector( void ) { return selector; }
        
		/* Methods */
		void                            addSocket(const Server *);
		void                            processEvents (const std::vector<Server*> & servers);
        bool                            isClientFD(int fd);
        bool                            isServerSocket(int fd, int serverSocket);
        bool                            isResponsePipe(int event_fd);
        void                            setClientFdEvent(int event_fd, int action);
        void                            addCgiProcessInfo(int clientFd, cgiProcessInfo CgiProcess);


        /* Getters */
        epoll_event*                    getEvents();
        std::set<int>&                  getActiveClients();
        std::map<int, cgiProcessInfo>&  getCgiProcessInfo();
        std::map<int, ConfigServer>&    getClientConfig();
        std::map<int, std::string>&     getRequests();
        int&                            getEpollFD();

};

#endif
