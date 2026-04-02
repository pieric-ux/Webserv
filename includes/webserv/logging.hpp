// TODO: don't forget header

#ifndef WEBSERV_LOGGING_HPP
#define WEBSERV_LOGGING_HPP

#include <log42/Log42.hpp>

#ifndef WEBSERV_LOG_DIR
# define WEBSERV_LOG_DIR "logs"
#endif

#define LOG_FMT "[%(asctime)] - %(name) - %(levelname) - %(message) - file: %(filename) - function: %(funcName)() at line: %(lineno)"
#define LOG_DATE_FMT "%Y-%m-%d %H:%M:%S,%(msecs) %Z"

#endif // !WEBSERV_LOGGING_HPP
