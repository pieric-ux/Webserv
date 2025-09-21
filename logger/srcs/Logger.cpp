// TODO: Don't forget to add 42 header !

/**
 * @file
 * @brief
 */

#include "Logger.hpp"
#include <ctime>
#include <cstring>
#include <string>

namespace logger
{

/**
 * @brief 
 *
 * @param filename 
 */
Logger::Logger(const std::string &init_filename) : filename(init_filename)
{
	this->logFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	this->logFile.open(this->filename.c_str());
}

/**
 * @brief 
 */
Logger::~Logger()
{
	this->logFile.close();
}

/**
 * @brief 
 *
 * @return 
 */
Logger &Logger::getGeneralLogger()
{
	static Logger general("general.log");

	return (general);
}

/**
 * @brief 
 */
void	Logger::displayTimestamp()
{
	std::time_t now = std::time(0);
	std::tm	*local_time = std::localtime(&now);
	std::string	buffer (16, 0);
	std::strftime(&buffer[0], buffer.size(), "[%Y-%m-%d %H:%M:%S]", local_time);
	buffer.resize(std::strlen(&buffer[0]));
	this->logFile << buffer;
}

/**
 * @brief 
 *
 * @param level 
 * @param msg 
 */
void	Logger::log(e_LogLevel level, const std::string &msg)
{
	if (!this->logFile.is_open())
		throw std::runtime_error("Unable to write to log file `" + this->filename + "`");

	this->displayTimestamp();
	this->logFile << "[" << this->levelToString(level) << "] " << msg << std::endl;
}

/**
 * @brief 
 *
 * @param level 
 */
std::string	Logger::levelToString(e_LogLevel level)
{
	switch (level)
	{
		case DEBUG: return ("DEBUG");
		case INFO: return ("INFO");
		case WARNING: return ("WARNING");
		case ERROR: return ("ERROR");
		case CRITICAL: return ("CRITICAL");
		default: return ("UNKNOWN");
	}
}

} // !logger
