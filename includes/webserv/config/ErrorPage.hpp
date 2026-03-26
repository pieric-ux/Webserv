// TODO: don't forget header

#ifndef WEBSERV_CONFIG_ERRORPAGE_HPP
#define WEBSERV_CONFIG_ERRORPAGE_HPP

/**
 * @file ErrorPage.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <string>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace config
{

class ErrorPage
{
	public:
		ErrorPage();
		ErrorPage(const std::vector<status::StatusCode> &codes, const std::string &path);
		~ErrorPage();

		ErrorPage(const ErrorPage &rhs);
		ErrorPage &operator=(const ErrorPage &rhs);

	private:
		t_Logger 						_logger;
		std::vector<status::StatusCode> _codes;
		std::string						_path;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_ERRORPAGE_HPP
