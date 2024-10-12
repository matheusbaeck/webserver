// #include "ServerManager.hpp"


// // void	ServerManager::WorkerIterator::forEachWorker( void (*f)( Worker & ) )
// // {
// // 	for (; *this != WorkerIterator(endServ, endServ); ++(*this))
// // 		f(**this);
// // }

// // void	ServerManager::WorkerIterator::forEachWorker( void (*f)( Worker & , void *param ), void *param )
// // {
// // 	for (; *this != WorkerIterator(endServ, endServ); ++(*this))
// // 		f(**this, param);
// // }

// bool ServerManager::WorkerIterator::operator!=( WorkerIterator& other) const
// {
// 	return (currServ != other.currServ || curr != other.curr);
// }

// Worker& ServerManager::WorkerIterator::operator*( void ) const { return (*curr); }

// ServerManager::WorkerIterator& ServerManager::WorkerIterator::operator++( void ) 


// /* division */

// ServerManager::ConstWorkerIterator::ConstWorkerIterator(ServerIterator startServer, ServerIterator endServ) : currServ(startServer), endServ(endServ)
// {
// 	if (currServ != endServ)
// 	{
// 		curr = currServ->workersBegin();
// 		advanceToNextValid();
// 	}
// }

// void ServerManager::ConstWorkerIterator::advanceToNextValid()
// {
// 	while (currServ != endServ && curr == currServ->workersEnd())
// 	{
// 		++currServ;
// 		if (currServ != endServ)
// 			curr = currServ->workersBegin();
// 	}
// }

// ServerIterator	ServerManager::ConstWorkerIterator::curServer( void )
// {
// 	return (this->currServ);
// }

// ServerIterator	ServerManager::ConstWorkerIterator::endServer( void )
// {
// 	return (this->endServ);
// }

// bool ServerManager::ConstWorkerIterator::operator!=(const ConstWorkerIterator& other) const
// {
// 	return (currServ != other.currServ || curr != other.curr);
// }

// const Worker& ServerManager::ConstWorkerIterator::operator*( void ) const { return (*curr); }

// ServerManager::ConstWorkerIterator& ServerManager::ConstWorkerIterator::operator++( void ) 
// {
// 	++curr;
// 	advanceToNextValid();
// 	return (*this);
// }