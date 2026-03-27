// TODO: don't forget header

#ifndef WEBSERV_CONFIG_ERRORPAGE_HPP
#define WEBSERV_CONFIG_ERRORPAGE_HPP

/**
 * @file ErrorPage.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace config
{

class ErrorPage
{
	public:
		ErrorPage();
		ErrorPage(const t_StatusCodes &codes, const std::string &path);
		~ErrorPage();

		ErrorPage(const ErrorPage &rhs);
		ErrorPage &operator=(const ErrorPage &rhs);

		t_Logger			getLogger() const;

	private:
		t_Logger 			_logger;
		t_StatusCodes		_codes;
		std::string			_path;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_ERRORPAGE_HPP
