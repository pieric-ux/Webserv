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
HTTPError::HTTPError() : _statusCode() 
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "HTTPError instance created with default constructor");
}

/**
 * @brief [TODO:description]
 *
 * @param statusCode [TODO:parameter]
 */
HTTPError::HTTPError(unsigned short code)
	:	_statusCode(status::StatusCodeRegistry::getInstance().getStatusCode(code)),
		_what(common::core::utils::toString(code) + " " + _statusCode.getMessage() + ": " + _statusCode.getDescription())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "HTTPError instance created: " + _what);
}

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
		_statusCode(rhs._statusCode),
		_what(rhs._what)
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
		_what = rhs._what;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	HTTPError::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
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
	return _what.c_str();
}

} // !client
} // !webserv
