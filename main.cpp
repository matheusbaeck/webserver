#include "Worker.hpp"
#include "Request.hpp"

int main( void )
{
	Worker					worker; /* use some container to have many workers (this can be handled in the Server class)*/
	std::queue<Request>		requests;

	std::cout << "Server listening to 127.0.0.1:" << SERVER_PORT << std::endl;
	for (;;) {
		/*for each worker*/
		Selector::getSelector().putEventsToQ(worker, requests);
		while (requests.size() > 0)
		{
			requests.front().handler();
			requests.pop();
		}
	}
	return 0;
}
