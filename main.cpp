#include "Worker.hpp"


void	hellow_world(int fd)
{
	char buffer[BUFFERSIZE];
	ssize_t bytes_read;

	bytes_read = read(fd, buffer, sizeof(buffer));
	if (bytes_read <= 0)
	{
		if (bytes_read == 0)
		{
			close(fd);
		}
		else
		{
			perror("read");
		}
		return ;
	}
	const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 12\r\n"
                           "\r\n"
                           "Hello World!";
    write(fd, response, strlen(response));
	//close(fd);
}

int main( void )
{
    Worker              worker1;
    std::queue<int>     requests;

    for (;;) {
        worker1.run(requests);
        while (requests.size() > 0)
        {
            hellow_world(requests.front());
            requests.pop();
        }
    }
    return 0;
}
