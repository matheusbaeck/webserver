# Self Hosted Webserver

This program attempts to recreate a HTTP/1.1 compliant web server able to fully serve a static website.<br> This server accepts GET, POST and DELETE methods from clients. It makes use of epoll, a Linux kernel feature, for I/O multiplexing. Our subject disallowed the use of threads and demanded the server to be non-blocking. In addition, we implemented CGI scripts and custom error pages. Our program made use of one epoll instance shared between all potential servers. Every read or write operation had to pass first by an epoll call.<br>

Since our web server had to be compliant with HTTP/1.1, all clients used the Connection: keep-alive header by default. Connections were closed, and clients were subsequently removed in the following cases:
1. When an error occurred during read, write, send, or recv system calls.
2. When specific HTTP errors, such as 403 (Forbidden), 413 (Payload Too Large), or 500 (Internal Server Error), were encountered. In these cases, the response included a Connection: close header to inform the client that the connection would not be reused.<br>

Our self-hosted web server used NGINX as a standard. We choose to implement only a subset of all available HTTP codes and focused on the most recurrent errors to craft our error pages and messages, see **err_pages/** for a list of all error_codes. Our program enable the servers it creates to listen to various ports at the same, yet ports cannot be reused in a server or even between two or more servers. The body size, a client can send to our servers, can also be limited by editing the **webserv.conf** file and changing the <ins>client_max_body directive</ins>.

## Installation

```bash
git clone https://github.com/matheusbaeck/webserver
cd webserver && make
./webserv
```

### Improvements I would have liked to make

Coming from C, my code is still very much function oriented. I wished I had created from the start a Request class. It would have looked something like this:
```cpp
class Request
{
    private:
        int clientFd;
        std::time_t time;
        std::string statusLine;
        std::string headers;
        std::string body;
        CgiHandler *script;
    public:
        //setters and getters that you can imagine
}
```

This class would have enabled me to regroup all information of a request and reduce the number of maps within my **selector** object. I also wished I had mapped a <ins>clientFd with a dequeue of Requests</ins>, I would have been able to receive multiple requests from the same client and treat them all sequentienly.

I would have liked to implement Automatas for the parsing of the **ConfigFile** and the **Request**. My friend [Jorge](https://github.com/JorgeVJ) showed me and his method seemed much more efficient and organized.

#### References

* [Official RFC HTTP1.1](https://datatracker.ietf.org/doc/html/rfc2616)
* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)
* [Epoll Linux Manual](https://man7.org/linux/man-pages/man7/epoll.7.html)
* [Stanford Network Programming Slides](https://web.stanford.edu/class/archive/cs/cs110/cs110.1202/static/lectures/19-events-threads.pdf)
* [Non-Blocking I/O youtube video](https://youtu.be/wB9tIg209-8?feature=shared)
* [Mozilla's resources about HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
* [HTTP and CGI explained](https://www.garshol.priv.no/download/text/http-tut.html)



