// TODO: don't forget header

#ifndef WEBSERV_STATUS_STATUSCODE_HPP
#define WEBSERV_STATUS_STATUSCODE_HPP

/**
* @file StatusCode.hpp
* @brief [TODO:description]
*/

#include <string>
#include <webserv/types.hpp>

namespace webserv
{
namespace status
{

/**
 * @class StatusCode
 * @brief [TODO:description]
 */
class StatusCode
{
	public:
		StatusCode();
		StatusCode(const unsigned short code, const std::string msg, const std::string description);
		~StatusCode();

		StatusCode(const StatusCode &rhs);
		StatusCode &operator=(const StatusCode &rhs);

		int				getCode() const;
		std::string		getMessage() const;
		std::string		getDescription() const;

	private:
		t_Logger		_logger;
		unsigned short	_code;
		std::string		_msg;
		std::string		_description;
};

} // !status
} // !webserv

#endif // !WEBSERV_STATUS_STATUSCODE_HPP
