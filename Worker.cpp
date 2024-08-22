#include "Worker.hpp"

Worker::Worker( void )
{
	this->_addr.sin_family = AF_INET;
	this->_addr.sin_port = htons(SERVER_PORT);
	this->_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

Worker::~Worker()
{
}