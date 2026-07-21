/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPError.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file HTTPError.cpp
 * @brief Implementation of the HTTPError exception, an std::exception carrying an HTTP status code, a formatted message and an optional redirect location.
 */

#include <webserv/client/HTTPError.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief Constructs an HTTPError with a default-initialized status code and acquires its logger.
 */
HTTPError::HTTPError() : _statusCode() 
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "HTTPError instance created with default constructor");
}

/**
 * @brief Constructs an HTTPError from a numeric HTTP status code, resolving it through the StatusCodeRegistry and building a "<code> <message>: <description>" what() string.
 *
 * @param code The numeric HTTP status code (e.g. 404) used to look up the matching StatusCode.
 */
HTTPError::HTTPError(unsigned short code)
	:	_statusCode(status::StatusCodeRegistry::getInstance().getStatusCode(code)),
		_what(common::core::utils::toString(code) + " " + _statusCode.getMessage() + ": " + _statusCode.getDescription())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "HTTPError instance created: " + _what);
}

HTTPError::HTTPError(unsigned short code, const std::string &location)
	:	_statusCode(status::StatusCodeRegistry::getInstance().getStatusCode(code)),
		_what(common::core::utils::toString(code) + " " + _statusCode.getMessage() + ": " + _statusCode.getDescription()),
		_location(location)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "HTTPError instance created: " + _what + " -> " + _location);
}

/**
 * @brief Destroys the HTTPError; declared throw() to honor the std::exception no-throw contract.
 */
HTTPError::~HTTPError() throw() {}

/**
 * @brief Copy-constructs an HTTPError, duplicating the base std::exception, status code, what() message and location.
 *
 * @param rhs The HTTPError instance to copy from.
 */
HTTPError::HTTPError(const HTTPError &rhs)
	:	std::exception(rhs),
		_statusCode(rhs._statusCode),
		_what(rhs._what),
		_location(rhs._location)
{}

/**
 * @brief Copy-assigns from another HTTPError, with a self-assignment guard, copying the base std::exception, status code, what() message and location.
 *
 * @param rhs The HTTPError instance to assign from.
 * @return A reference to this HTTPError.
 */
HTTPError &HTTPError::operator=(const HTTPError &rhs)
{
	if (this != &rhs)
	{
		std::exception::operator=(rhs);
		_statusCode = rhs._statusCode;
		_what = rhs._what;
		_location = rhs._location;
	}
	return (*this);
}

/**
 * @brief Returns the shared logger for the HTTPError class, retrieved from the log42 manager under "webserv.client.HTTPError".
 *
 * @return The logger associated with the HTTPError class.
 */
t_Logger	HTTPError::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.HTTPError");
}

/**
 * @brief Returns the HTTP status code carried by this error.
 *
 * @return A copy of the StatusCode associated with this error.
 */
status::StatusCode HTTPError::getStatusCode() const
{
	return _statusCode;
}

/**
 * @brief Replaces the HTTP status code carried by this error.
 *
 * @param statusCode The new StatusCode to store in this error.
 */
void HTTPError::setStatusCode(const status::StatusCode &statusCode)
{
	_statusCode = statusCode;
}

/**
 * @brief Returns the optional redirect location associated with this error.
 *
 * @return A const reference to the location string, empty when no location was provided.
 */
const std::string &HTTPError::getLocation() const
{
	return _location;
}

/**
 * @brief Returns the human-readable error message, overriding std::exception::what().
 *
 * @return A C string of the form "<code> <message>: <description>" describing the error.
 */
const char *HTTPError::what() const throw()
{
	return _what.c_str();
}

} // !client
} // !webserv
