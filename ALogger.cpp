#include "ALogger.hpp"

ALogger::ALogger() { logger = Log::getInstance(LOG_FILE); }

ALogger::~ALogger() {}

std::ostream	&operator<<( std::ostream &os, const ALogger &l )
{
	os << l.GetType() << l;
	return (os);
}

std::string ALogger::GetType() const
{
	return "ALogger";
}