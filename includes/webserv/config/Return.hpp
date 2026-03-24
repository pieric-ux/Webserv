// TODO: don't forget header

#ifndef WEBSERV_CONFIG_RETURN_HPP
#define WEBSERV_CONFIG_RETURN_HPP

/**
 * @file Return.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace config
{

struct Return
{
	StatusCode	statusCode;
	std::string	url;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_RETURN_HPP
