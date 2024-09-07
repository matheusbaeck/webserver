#include "Worker.hpp"
#include "Request.hpp"

int main( void )
{
	std::cout << "Server listening to 127.0.0.1:" << std::endl;
	Worker					worker; /* use some container to have many workers (this can be handled in the Server class)*/
	Worker					worker2(3000);
	std::queue<Request>		requests;

	for (;;) {
		/*for each worker*/
		Selector::getSelector().putEventsToQ(worker2, requests);
		Selector::getSelector().putEventsToQ(worker, requests);
		while (requests.size() > 0)
		{
			requests.front().handler();
			requests.pop();
		}
	}
	return 0;
}
