// TODO: don't forget header

#ifndef WEBSERV_STATUSCODEREGISTRY_HPP
#define WEBSERV_STATUSCODEREGISTRY_HPP

/**
* @file StatusCodeRegistry.hpp
* @brief
*/

#include <webserv/StatusCode.hpp>
#include <map>

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
		static StatusCodeRegistry &getInstance();
		StatusCode getStatusCode(unsigned short code);

	private:
		std::map<unsigned short, StatusCode> _statusCodes;

		StatusCodeRegistry();
		~StatusCodeRegistry();
		StatusCodeRegistry(const StatusCodeRegistry &rhs);
		StatusCodeRegistry &operator=(const StatusCodeRegistry &rhs);
};

} // !status

#endif
