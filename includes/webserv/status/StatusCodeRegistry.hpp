// TODO: don't forget header

#ifndef WEBSERV_STATUS_STATUSCODEREGISTRY_HPP
#define WEBSERV_STATUS_STATUSCODEREGISTRY_HPP

/**
* @file StatusCodeRegistry.hpp
* @brief [TODO:description]
*/

#include <map>
#include <webserv/status/StatusCode.hpp>
#include <webserv/types.hpp>

namespace webserv
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
		static StatusCodeRegistry	&getInstance();

		t_Logger					getLogger() const;

		StatusCode					getStatusCode(const unsigned short code) const;


	private:
		t_Logger					_logger;
		t_StatusCodesRegistry		_statusCodes;

		StatusCodeRegistry();
		~StatusCodeRegistry();
		StatusCodeRegistry(const StatusCodeRegistry &rhs);
		StatusCodeRegistry &operator=(const StatusCodeRegistry &rhs);
};

} // !status
} // !webserv

#endif // !WEBSERV_STATUS_STATUSCODEREGISTRY_HPP
