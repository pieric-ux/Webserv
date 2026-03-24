// TODO: don't forget header

#ifndef WEBSERV_STATUS_STATUSCODEREGISTRY_HPP
#define WEBSERV_STATUS_STATUSCODEREGISTRY_HPP

/**
* @file StatusCodeRegistry.hpp
* @brief [TODO:description]
*/

#include <webserv/status/StatusCode.hpp>
#include <map>

namespace http
{
namespace status
{

/**
 * @class StatusCodeRegistry
 * @brief [TODO:description]
 *
 * @link https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
 */
class StatusCodeRegistry
{
	public:
		static StatusCodeRegistry				&getInstance();
		StatusCode								getStatusCode(const unsigned short code) const;

	private:
		std::map<unsigned short, StatusCode>	_statusCodes;

		StatusCodeRegistry();
		~StatusCodeRegistry();
		StatusCodeRegistry(const StatusCodeRegistry &rhs);
		StatusCodeRegistry &operator=(const StatusCodeRegistry &rhs);
};

} // !status
} // !http

#endif // !WEBSERV_STATUS_STATUSCODEREGISTRY_HPP
