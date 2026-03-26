// TODO: don't forget header

#ifndef WEBSERV_HEADERS_HTTPHEADERSREGISTRY_HPP
#define WEBSERV_HEADERS_HTTPHEADERSREGISTRY_HPP

/**
* @file HTTPHeadersRegistry.hpp
* @brief [TODO:description]
*/

#include <string>
#include <map>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/types.hpp>
namespace webserv
{
namespace HTTPheaders
{

/**
 * @class HTTPHeadersRegistry
 * @brief [TODO:description]
 *
 * @link https://www.iana.org/assignments/http-fields/http-fields.xhtml
 */
class HTTPHeadersRegistry
{
	public:
		static HTTPHeadersRegistry &getInstance();
		HTTPHeader getHeader(const std::string name) const;

	private:
		t_Logger							_logger;
		std::map<std::string, HTTPHeader>	_headers;

		HTTPHeadersRegistry();
		~HTTPHeadersRegistry();
		HTTPHeadersRegistry(const HTTPHeadersRegistry &rhs);
		HTTPHeadersRegistry &operator=(const HTTPHeadersRegistry &rhs);
};

} // !HTTPheaders
} // !webserv

#endif // !WEBSERV_HEADERS_HTTPHEADERSREGISTRY_HPP
