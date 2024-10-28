#ifndef LOG_HPP__
# define LOG_HPP__

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <ctime>

enum LogLevel
{
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
	NONE
};

# define OUTFILE_LOG_LEVEL TRACE

# ifndef COUT_LOG_LEVEL
#  define COUT_LOG_LEVEL ERROR
# endif

class Log
{
	private:
		static Log* logger;
		std::ofstream outFile;
		bool fileAvailable;
		const char* strLevel[6];
		const char* colors[6];
		const char* reset;

		Log(const std::string& );
		
		void outputLog( const void* , int , const std::string& );

	public:
		~Log();

		// static Log* getInstance(const std::string& filePath = "default.log")
		// {
		// 	if (instance == NULL) {
		// 		instance = new Log(filePath);
		// 	}
		// 	return instance;
		// }

		// static Log* getInstance(const std::string& filePath);

		static Log* getInstance(const std::string& filePath)
		{
			static Log logger(filePath);
			return &logger;
		}

		void logMessage( const void* entity, int logLevel, const std::string& message, std::exception* ex = NULL)
		{
			if (logLevel == NONE) return;
			std::ostringstream oss;
			oss << message;

			if (ex)
			{
				oss << " | Exception: " << ex->what();
			}
			outputLog(entity, logLevel, oss.str());
		}

		// template <typename T>
		// void logMessage( const void* entity, int logLevel, const std::string& message, const T& arg, std::exception* ex = NULL)
		// {
		// 	std::ostringstream oss;
		// 	oss << message << ": " << arg;

		// 	if (ex)
		// 	{
		// 		oss << " | Exception: " << ex->what();
		// 	}
		// 	outputLog(entity, logLevel, oss.str());
		// }

		// template <typename T, typename... Args>
		// void logMessage( const void* entity, int logLevel, const std::string& message, const T& firstArg, const Args&... args)
		// {
		// 	if (logLevel >= currentLevel) {
		// 		std::ostringstream oss;
		// 		oss << message << ": " << firstArg;
		// 		(logMessage(entity, logLevel, oss.str(), args...), ...);
		// 	}
		// }

		// template <typename T>
		// void logMessage( const void* entity, int logLevel, const std::string& message, typename T::iterator it, typename T::iterator end, std::exception* ex = NULL)
		// {
		// 	std::ostringstream oss;
		// 	oss << message << ": ";

		// 	for (; it != end; ++it) {
		// 		oss << *it << " ";
		// 	}

		// 	if (ex) {
		// 		oss << "| Exception: " << ex->what();
		// 	}

		// 	outputLog(entity, logLevel, oss.str());
		// }
};

#endif
