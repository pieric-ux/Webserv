// TODO: don't forget header

/**
* @file StatusCode.cpp
* @brief
*/

#include <webserv/StatusCode.hpp>

namespace status
{

/**
 * @brief [TODO:description]
 */
StatusCode::StatusCode() : _code(0), _msg(), _description() {}

/**
 * @brief [TODO:description]
 *
 * @param code [TODO:parameter]
 * @param msg [TODO:parameter]
 * @param description [TODO:parameter]
 */
StatusCode::StatusCode(unsigned short code, std::string msg, std::string description) : _code(code), _msg(msg), _description(description) {}

/**
 * @brief [TODO:description]
 */
StatusCode::~StatusCode() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
StatusCode::StatusCode(const StatusCode &rhs) : _code(rhs._code), _msg(rhs._msg), _description(rhs._description) {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
StatusCode &StatusCode::operator=(const StatusCode &rhs)
{
	if (this != &rhs)
	{
		_code = rhs._code;
		_msg = rhs._msg;
		_description = rhs._description;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int StatusCode::getCode()
{
	return (_code);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string StatusCode::getMessage()
{
	return (_msg);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string StatusCode::getDescription()
{
	return (_description);
}

} // !status
