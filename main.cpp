#include "Worker.hpp"
#include "Request.hpp"

int main( void )
{
	Worker					worker; /* use some container to have many workers (this can be handled in the Server class)*/
	std::queue<Request>		requests;

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
