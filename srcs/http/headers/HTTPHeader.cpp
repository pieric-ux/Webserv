// TODO: don't forget header

/**
* @file HTTPHeaders.cpp
* @brief [TODO:description]
*/

#include "webserv/HTTPHeader.hpp"
#include <string>

namespace http
{
namespace HTTPheaders
{

/**
 * @brief [TODO:description]
 */
HTTPHeader::HTTPHeader() : _name(), _value(), _description() {}

/**
 * @brief [TODO:description]
 *
 * @param name [TODO:description]
 * @param description [TODO:description]
 */
HTTPHeader::HTTPHeader(const std::string name, const std::string description) : _name(name), _value(""), _description(description) {}

/**
 * @brief [TODO:description]
 *
 * @param name [TODO:description]
 * @param value [TODO:description]
 * @param description [TODO:description]
 */
HTTPHeader::HTTPHeader(const std::string name, const std::string value, const std::string description) : _name(name), _value(value), _description(description) {}

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
HTTPHeader &HTTPHeader::operator=(const HTTPHeader &rhs)
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
 * @return [TODO:description]
 */
std::string HTTPHeader::getName() const
{
	return (_name);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
std::string HTTPHeader::getValue() const
{
	return (_value);
}

/**
 * @brief [TODO:description]
 *
 * @param value [TODO:description]
 * @return [TODO:description]
 */
void HTTPHeader::setValue(const std::string value)
{
	_value = value;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
std::string HTTPHeader::getDescription() const
{
	return _description;
}

} // !HTTPheaders
} // !http
