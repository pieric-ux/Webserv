// TODO: don't forget header

/**
* @file HTTPHeaders.cpp
* @brief [TODO:description]
*/

#include <string>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace HTTPheaders
{

/**
 * @brief [TODO:description]
 */
HTTPHeader::HTTPHeader() : _name(), _value(), _description()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
	_logger->setLevel(log42::logRecord::INFO);
	INFO (_logger, "HTTPHeader instance created with default constructor");
}

/**
 * @brief [TODO:description]
 *
 * @param name [TODO:description]
 * @param description [TODO:description]
 */
HTTPHeader::HTTPHeader(const std::string name, const std::string description) : _name(name), _value(""), _description(description) 
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
	_logger->setLevel(log42::logRecord::INFO);
	INFO (_logger, "HTTPHeader instance created with name: " + name + " and description: " + description);
}

/**
 * @brief [TODO:description]
 *
 * @param name [TODO:description]
 * @param value [TODO:description]
 * @param description [TODO:description]
 */
HTTPHeader::HTTPHeader(const std::string name, const std::string value, const std::string description) : _name(name), _value(value), _description(description) 
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
	_logger->setLevel(log42::logRecord::INFO);
	INFO (_logger, "HTTPHeader instance created with name: " + name + ", value: " + value + " and description: " + description);
}

/**
 * @brief [TODO:description]
 */
HTTPHeader::~HTTPHeader () {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:description]
 */
HTTPHeader::HTTPHeader(const HTTPHeader &rhs) : _name(rhs._name), _value(rhs._value), _description(rhs._description) {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:description]
 * @return [TODO:description]
 */
HTTPHeader	&HTTPHeader::operator=(const HTTPHeader &rhs)
{
	if (this != &rhs)
	{
		_name = rhs._name;
		_value = rhs._value;
		_description = rhs._description;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	HTTPHeader::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.headers.httpheader");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
const std::string	&HTTPHeader::getName() const
{
	return (_name);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
const std::string	&HTTPHeader::getValue() const
{
	return (_value);
}

/**
 * @brief [TODO:description]
 *
 * @param value [TODO:description]
 * @return [TODO:description]
 */
void	HTTPHeader::setValue(const std::string value)
{
	_value = value;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
const std::string	&HTTPHeader::getDescription() const
{
	return _description;
}

} // !HTTPheaders
} // !webserv
