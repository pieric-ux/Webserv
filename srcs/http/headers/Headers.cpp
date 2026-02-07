// TODO: don't forget header

/**
* @file Headers.cpp
* @brief [TODO:description]
*/

#include "webserv/Headers.hpp"
#include <string>
namespace http
{
namespace headers
{

/**
 * @brief [TODO:description]
 */
Headers::Headers() : _name(), _description() {}

/**
 * @brief [TODO:description]
 *
 * @param name [TODO:description]
 * @param description [TODO:description]
 */
Headers::Headers(const std::string name, const std::string description) : _name(name), _description(description) {}

/**
 * @brief [TODO:description]
 */
Headers::~Headers () {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:description]
 */
Headers::Headers(const Headers &rhs) : _name(rhs._name), _description(rhs._description) {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:description]
 * @return [TODO:description]
 */
Headers &Headers::operator=(const Headers &rhs)
{
	if (this != &rhs)
	{
		_name = rhs._name;
		_description = rhs._description;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
std::string Headers::getName() const
{
	return (_name);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:description]
 */
std::string Headers::getDescription() const
{
	return _description;
}

} // !headers
} // !http
