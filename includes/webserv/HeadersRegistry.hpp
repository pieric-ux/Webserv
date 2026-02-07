// TODO: don't forget header

#ifndef WEBSERV_HEADERSREGISTRY_HPP
#define WEBSERV_HEADERSREGISTRY_HPP

/**
* @file HeaderRegistry.hpp
* @brief [TODO:description]
*/

#include <webserv/Headers.hpp>
#include <string>
#include <map>


namespace http
{
namespace headers
{

/**
 * @class HeadersRegistry
 * @brief [TODO:description]
 *
 * @link https://www.iana.org/assignments/http-fields/http-fields.xhtml
 */
class HeadersRegistry
{
	public:
		static HeadersRegistry &getInstance();
		Headers getHeader(const std::string name) const;

	private:
		std::map<std::string, Headers> _headers;

		HeadersRegistry();
		~HeadersRegistry();
		HeadersRegistry(const HeadersRegistry &rhs);
		HeadersRegistry &operator=(const HeadersRegistry &rhs);
};

} // !headers
} // !http

#endif
