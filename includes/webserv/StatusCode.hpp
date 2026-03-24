// TODO: don't forget header

#ifndef WEBSERV_STATUSCODE_HPP
#define WEBSERV_STATUSCODE_HPP

/**
* @file StatusCode.hpp
* @brief [TODO:description]
*/

#include <string>

namespace http
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

		int			getCode() const;
		std::string	getMessage() const;
		std::string	getDescription() const;

	private:
		unsigned short		_code;
		std::string			_msg;
		std::string			_description;
};

} // !status
} // !http

#endif
