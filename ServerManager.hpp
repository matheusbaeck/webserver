#ifndef SERVERMANAGER_HPP__
# define SERVERMANAGER_HPP__

#include <vector>
#include <sstream>

#include "ALogger.hpp"
#include "Server.hpp"

class Server;

typedef std::vector<Server>::iterator ServerIterator;
typedef std::vector<Worker>::iterator WorkerIteratorStd;

class ServerManager : public ALogger
{
	private:
		std::vector<Server> servers;
		std::queue<Request>	requests;

		class WorkerIterator
		{
			private:
				ServerIterator		currServ;
				ServerIterator		endServ;
				WorkerIteratorStd	curr;

				void advanceToNextValid()
				{
					while (currServ != endServ && (curr == currServ->workersEnd() || currServ->getWorkers().empty()))
					{
						++currServ;
						if (currServ != endServ)
							curr = currServ->workersBegin();
					}
				}

			public:
				WorkerIterator(ServerIterator startServer, ServerIterator endServ)
					: currServ(startServer), endServ(endServ)
				{
					if (currServ != endServ)
					{
						curr = currServ->workersBegin();
						advanceToNextValid();
					}
				}

				bool	operator!=( const WorkerIterator& other ) const
				{
					return (currServ != other.currServ || (currServ != endServ && curr != other.curr));
				}

				WorkerIterator& operator++()
				{
					if (currServ != endServ)
					{
						++curr;
						advanceToNextValid();
					}
					return (*this);
				}

				Worker& operator*(void)
				{
					return (*curr);
				}

				const Worker& operator*(void) const
				{
					return (*curr);
				}

				bool operator==(const WorkerIterator& other) const
				{
					return !(*this != other);
				}

				WorkerIterator operator++(int)
				{
					WorkerIterator tmp = *this;
					++(*this);
					return tmp;
				}

				Worker* operator->()
				{
					return &(*curr);
				}
		};

	public:
		ServerManager( void );
		ServerManager( const std::string );
		ServerManager( std::vector<std::vector<int> > );
		~ServerManager( void );

		/* Operator */
		ServerManager& operator=(const ServerManager& other);

		/* Acessors */
		std::vector<Server> 	&getServers( void );
		std::queue<Request>		&getQueue( void );
		WorkerIterator begin()
		{
			return WorkerIterator(servers.begin(), servers.end());
		}

		WorkerIterator end()
		{
			return WorkerIterator(servers.end(), servers.end());
		}

		/* Methods */
		void							RequestHandler( void );
		void							addServer( const Server &server );
		std::vector<std::vector<int> >	createVector(const std::string& data);

		/* Templates */
		template<typename Func>
		void forEachWorker(Func f)
		{
			for (WorkerIterator it = begin(); it != end() ; ++it) {
				f(*it);
			}
		}

		template <typename Func>
		void forEachWorkerAndQueue(Func f)
		{
			for (WorkerIterator it = begin(); it != end() ; ++it) {
				f(*it, this->requests);
			}
		}

		/* Functors */
		class AddServerFunctor {
			private:
				ServerManager* serverManager;
			public:
				AddServerFunctor(ServerManager* sm) : serverManager(sm) {}

				void operator()(const std::vector<int>& ports) {
					serverManager->addServer(Server(ports));
				}
		};

		/* Logger */
		void				LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL);
		void				LogMessage(int logLevel, std::exception* ex = NULL);
		virtual std::string	GetType() const;
};

std::vector<std::vector<int> > createVector(const std::string& data);

#endif