/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusCode.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
* @file StatusCode.cpp
* @brief Implements the StatusCode value object that bundles an HTTP status code with its reason phrase and description, providing constructors, copy semantics and read-only accessors.
*/

#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace status
{

/**
 * @brief Default-constructs a StatusCode with a zero code and empty message and description, and initializes the logger.
 */
StatusCode::StatusCode() : _code(0), _msg(), _description()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.status.statuscode");
	_logger->setLevel(log42::logRecord::INFO);
	DEBUG(_logger, "StatusCode instance created with default constructor");
}

/**
 * @brief Constructs a StatusCode from an explicit code, reason-phrase message and description, and initializes the logger.
 *
 * @param code The numeric HTTP status code.
 * @param msg The reason-phrase message associated with the status code.
 * @param description A human-readable description of the status code.
 */
StatusCode::StatusCode(const unsigned short code, const std::string msg, const std::string description) : _code(code), _msg(msg), _description(description)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.status.statuscode");
	_logger->setLevel(log42::logRecord::INFO);
	DEBUG(_logger, "StatusCode instance created with code: " + common::core::utils::toString(code) + ", message: " + msg);
}

/**
 * @brief Destroys the StatusCode instance.
 */
StatusCode::~StatusCode() {}

/**
 * @brief Copy-constructs a StatusCode by copying the code, message and description of another instance.
 *
 * @param rhs The StatusCode instance to copy from.
 */
StatusCode::StatusCode(const StatusCode &rhs) : _code(rhs._code), _msg(rhs._msg), _description(rhs._description) {}

/**
 * @brief Copy-assigns the code, message and description from another StatusCode, guarding against self-assignment.
 *
 * @param rhs The StatusCode instance to assign from.
 * @return A reference to this StatusCode after assignment.
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
 * @brief Retrieves the logger associated with the StatusCode class from the logging manager.
 *
 * @return The logger registered under "webserv.status.statuscode".
 */
t_Logger	StatusCode::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.status.statuscode");
}

/**
 * @brief Returns the numeric HTTP status code.
 *
 * @return The stored status code value.
 */
int StatusCode::getCode() const
{
	return (_code);
}

/**
 * @brief Returns the reason-phrase message associated with the status code.
 *
 * @return A const reference to the stored message string.
 */
const std::string	&StatusCode::getMessage() const
{
	return (_msg);
}

/**
 * @brief Returns the human-readable description of the status code.
 *
 * @return A const reference to the stored description string.
 */
const std::string &StatusCode::getDescription() const
{
	return (_description);
}

} // !status
} // !webserv
