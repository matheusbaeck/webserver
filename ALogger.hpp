#ifndef ALOGGER_HPP__
# define ALOGGER_HPP__

#include "Log.hpp"

#define LOG_FILE "logfile.log"

class ALogger
{
	protected:
		Log* logger;
		std::ostringstream m_oss;
	public:
		ALogger();
		virtual ~ALogger();
		virtual void	LogMessage(int logLevel, const std::string& message, std::exception* ex = NULL) = 0;
		virtual void	LogMessage(int logLevel, std::exception* ex = NULL) = 0;
		virtual std::string GetType() const = 0;

		std::ostringstream &oss( void )
		{
			m_oss.str("");
			m_oss.clear();
			return m_oss;
		}
};

std::ostream	&operator<<( std::ostream &os, const ALogger &l );

#endif