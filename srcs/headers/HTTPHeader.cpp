/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeader.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
* @file HTTPHeaders.cpp
* @brief Implements the HTTPHeader class, modeling a single HTTP header field
* with its name, value, and human-readable description.
*/

#include <string>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace HTTPheaders
{

/**
 * @brief Default-constructs an empty HTTPHeader and acquires its logger.
 */
HTTPHeader::HTTPHeader() : _name(), _value(), _description()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
	_logger->setLevel(log42::logRecord::INFO);
	DEBUG(_logger, "HTTPHeader instance created with default constructor");
}

/**
 * @brief Constructs an HTTPHeader with a name and description, leaving the value empty.
 *
 * @param name The header field name (e.g. "Content-Type").
 * @param description Human-readable description of the header's purpose.
 */
HTTPHeader::HTTPHeader(const std::string name, const std::string description) : _name(name), _value(""), _description(description) 
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
	_logger->setLevel(log42::logRecord::INFO);
	DEBUG(_logger, "HTTPHeader instance created with name: " + name + " and description: " + description);
}

/**
 * @brief Constructs a fully populated HTTPHeader with a name, value, and description.
 *
 * @param name The header field name (e.g. "Content-Type").
 * @param value The header field value.
 * @param description Human-readable description of the header's purpose.
 */
HTTPHeader::HTTPHeader(const std::string name, const std::string value, const std::string description) : _name(name), _value(value), _description(description) 
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
	_logger->setLevel(log42::logRecord::INFO);
	DEBUG(_logger, "HTTPHeader instance created with name: " + name + ", value: " + value + " and description: " + description);
}

/**
 * @brief Destroys the HTTPHeader instance.
 */
HTTPHeader::~HTTPHeader () {}

/**
 * @brief Copy-constructs an HTTPHeader by duplicating another instance's fields.
 *
 * @param rhs The HTTPHeader to copy from.
 */
HTTPHeader::HTTPHeader(const HTTPHeader &rhs) : _logger(rhs._logger), _name(rhs._name), _value(rhs._value), _description(rhs._description) {}

/**
 * @brief Copy-assigns another HTTPHeader, copying its fields unless self-assigned.
 *
 * @param rhs The HTTPHeader to assign from.
 * @return Reference to this HTTPHeader after assignment.
 */
HTTPHeader	&HTTPHeader::operator=(const HTTPHeader &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_name = rhs._name;
		_value = rhs._value;
		_description = rhs._description;
	}
	return (*this);
}

/**
 * @brief Returns the logger associated with the HTTPHeader class.
 *
 * @return The "webserv.headers.httpheader" logger instance.
 */
t_Logger	HTTPHeader::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
}

/**
 * @brief Accesses the header field name.
 *
 * @return Const reference to the header's name.
 */
const std::string	&HTTPHeader::getName() const
{
	return (_name);
}

/**
 * @brief Accesses the header field value.
 *
 * @return Const reference to the header's value.
 */
const std::string	&HTTPHeader::getValue() const
{
	return (_value);
}

/**
 * @brief Sets the header field value.
 *
 * @param value The new value to store for this header.
 */
void	HTTPHeader::setValue(const std::string value)
{
	_value = value;
}

/**
 * @brief Accesses the header's human-readable description.
 *
 * @return Const reference to the header's description.
 */
const std::string	&HTTPHeader::getDescription() const
{
	return _description;
}

} // !HTTPheaders
} // !webserv
