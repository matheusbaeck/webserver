#include "Log.hpp"
#include "ALogger.hpp"

Log* Log::logger = NULL;

void Log::outputLog(const void* entity, int logLevel, const std::string& message)
{
	std::time_t now;
	std::ostringstream oss;
	char buffer[100];
	
	const ALogger* loggerEntity = static_cast<const ALogger*>(entity);
	now = std::time(NULL);
	std::strftime(buffer, sizeof(buffer), "%c", std::localtime(&now));
	oss << colors[logLevel]
		<< buffer
		<< " | " << std::setw(5) << strLevel[logLevel]
		<< " | ";
	try {
		if (entity) {
			oss << std::setw(15) << loggerEntity->GetType();
		} else {
			oss << std::setw(15) << "unknown";
		}
	} catch (const std::exception& e) {
		oss << std::setw(15) << "entity_error";
	}
	oss << " | " << message << reset;

	if (logLevel >= COUT_LOG_LEVEL)
		std::cout << oss.str() << std::endl;
	if (fileAvailable && logLevel >= OUTFILE_LOG_LEVEL)
		outFile << oss.str() << std::endl;
}

Log::Log(const std::string& filePath) : fileAvailable(false)
{
	outFile.open(filePath.c_str(), std::ios::app);
	if (outFile.is_open()) {
		fileAvailable = true;
	} else {
		std::cerr << "Failed to open or create log file." << std::endl;
	}
	strLevel[0] = "TRACE";
	strLevel[1] = "DEBUG";
	strLevel[2] = "INFO";
	strLevel[3] = "WARN";
	strLevel[4] = "ERROR";
	strLevel[5] = "FATAL";

	colors[0] = "\033[37m";  // TRACE - White
	colors[1] = "\033[34m";  // DEBUG - Blue
	colors[2] = "\033[32m";  // INFO  - Green
	colors[3] = "\033[33m";  // WARN  - Yellow
	colors[4] = "\033[31m";  // ERROR - Red
	colors[5] = "\033[35m";  // FATAL - Magenta
	reset = "\033[0m"; // Reset color
	}

Log::~Log()
{
	if (fileAvailable) {
		outFile.close();
	}
}
