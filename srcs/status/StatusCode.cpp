// TODO: don't forget header

/**
* @file StatusCode.cpp
* @brief [TODO:description]
*/

#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace status
{

/**
 * @brief [TODO:description]
 */
StatusCode::StatusCode() : _code(0), _msg(), _description()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.status.statuscode");
	_logger->setLevel(log42::logRecord::INFO);
}

/**
 * @brief [TODO:description]
 *
 * @param code [TODO:parameter]
 * @param msg [TODO:parameter]
 * @param description [TODO:parameter]
 */
StatusCode::StatusCode(const unsigned short code, const std::string msg, const std::string description) : _code(code), _msg(msg), _description(description) {}

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
int StatusCode::getCode() const
{
	return (_code);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string StatusCode::getMessage() const
{
	return (_msg);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string StatusCode::getDescription() const
{
	return (_description);
}

} // !status
} // !webserv
