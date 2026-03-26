// TODO: don't forget header

/**
 * @file HTTPError.cpp
 * @brief [TODO:description]
 */

#include <webserv/client/HTTPError.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief [TODO:description]
 */
HTTPError::HTTPError() : _statusCode() {}

/**
 * @brief [TODO:description]
 *
 * @param statusCode [TODO:parameter]
 */
HTTPError::HTTPError(const status::StatusCode &statusCode)
	:	_statusCode(statusCode)
{}

/**
 * @brief [TODO:description]
 */
HTTPError::~HTTPError() throw() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
HTTPError::HTTPError(const HTTPError &rhs)
	:	std::exception(rhs),
		_statusCode(rhs._statusCode)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
HTTPError &HTTPError::operator=(const HTTPError &rhs)
{
	if (this != &rhs)
	{
		std::exception::operator=(rhs);
		_statusCode = rhs._statusCode;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
status::StatusCode HTTPError::getStatusCode() const
{
	return _statusCode;
}

/**
 * @brief [TODO:description]
 *
 * @param statusCode [TODO:parameter]
 */
void HTTPError::setStatusCode(const status::StatusCode &statusCode)
{
	_statusCode = statusCode;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const char *HTTPError::what() const throw()
{
	return "";
}

} // !client
} // !webserv
