// TODO: Don't forget to add 42 header !

#ifndef LOGGER_HPP
#define LOGGER_HPP

/**
 * @file
 * @brief
 */

#include <string>
#include <fstream>

namespace logger
{

enum e_LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger
{
	public:
		explicit Logger(const std::string &filename);
		~Logger();

		static Logger &getGeneralLogger();

		void	displayTimestamp();
		void	log(e_LogLevel level, const std::string &msg);

	private:
		std::ofstream	logFile;
		std::string		filename;

		Logger(const Logger &rhs);
		Logger &operator=(const Logger &rhs);

		std::string levelToString(e_LogLevel level);
};

} // !logger
#endif
