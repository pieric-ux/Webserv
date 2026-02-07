// TODO: don't forget header

#ifndef WEBSERV_STATUSCODE_HPP
#define WEBSERV_STATUSCODE_HPP

/**
* @file StatusCode.hpp
* @brief
*/

#include <string>

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
		StatusCode(unsigned short code, std::string msg, std::string description);
		~StatusCode();

		StatusCode(const StatusCode &rhs);
		StatusCode &operator=(const StatusCode &rhs);

		int			getCode();
		std::string	getMessage();
		std::string	getDescription();

	private:
		unsigned short		_code;
		std::string			_msg;
		std::string			_description;
};

} // !status

#endif
