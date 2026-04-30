// TODO: don't forget header

/**
 * @file Cookie.cpp
 * @brief [TODO:description]
 */

#include <webserv/session/Cookie.hpp>

namespace webserv
{
namespace session
{

/**
 * @brief [TODO:description]
 *
 * @param name [TODO:parameter]
 * @param value [TODO:parameter]
 */
Cookie::Cookie(const std::string &name, const std::string &value)
	:	_name(name),
		_value(value),
		_expires(0),
		_path("/"),
		_domain(),
		_secure(false),
		_httpOnly(true)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.session.cookie");
    _logger->setLevel(log42::logRecord::DEBUG);
    INFO(_logger, "Cookie instance created");
}

/**
 * @brief [TODO:description]
 */
Cookie::~Cookie() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Cookie::Cookie(const Cookie &rhs)
	:	_logger(rhs._logger),
		_name(rhs._name),
		_value(rhs._value),
		_expires(rhs._expires),
		_path(rhs._path),
		_domain(rhs._domain),
		_secure(rhs._secure),
		_httpOnly(rhs._httpOnly)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
Cookie &Cookie::operator=(const Cookie &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_name = rhs._name;
		_value = rhs._value;
		_expires = rhs._expires;
		_path = rhs._path;
		_domain = rhs._domain;
		_secure = rhs._secure;
		_httpOnly = rhs._httpOnly;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger Cookie::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.session.cookie");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &Cookie::getName() const
{
	return _name;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &Cookie::getValue() const
{
	return _value;
}

/**
 * @brief [TODO:description]
 *
 * @param value [TODO:parameter]
 */
void Cookie::setValue(const std::string &value)
{
	_value = value;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::time_t Cookie::getExpires() const
{
	return _expires;
}

/**
 * @brief [TODO:description]
 *
 * @param expires [TODO:parameter]
 */
void Cookie::setExpires(std::time_t expires)
{
	_expires = expires;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &Cookie::getPath() const
{
	return _path;
}

/**
 * @brief [TODO:description]
 *
 * @param path [TODO:parameter]
 */
void Cookie::setPath(const std::string &path)
{
	_path = path;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &Cookie::getDomain() const
{
	return _domain;
}

/**
 * @brief [TODO:description]
 *
 * @param domain [TODO:parameter]
 */
void Cookie::setDomain(const std::string &domain)
{
	_domain = domain;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool Cookie::isSecure() const
{
	return _secure;
}

/**
 * @brief [TODO:description]
 *
 * @param secure [TODO:parameter]
 */
void Cookie::setSecure(bool secure)
{
	_secure = secure;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool Cookie::isHttpOnly() const
{
	return _httpOnly;
}

/**
 * @brief [TODO:description]
 *
 * @param httpOnly [TODO:parameter]
 */
void Cookie::setHttpOnly(bool httpOnly)
{
	_httpOnly = httpOnly;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool Cookie::isExpired() const
{
	if (_expires == 0)
		return false;
	return std::time(NULL) > _expires;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string Cookie::serializeCookie() const
{
	std::string result = _name + "=" + _value;

	if (!_path.empty())
		result += "; Path=" + _path;
	if (!_domain.empty())
		result += "; Domain=" + _domain;
	if (_expires != 0)
	{
		std::tm tm = *std::gmtime(&_expires);
		char buf[64];
		std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
		result += "; Expires=";
		result += buf;
	}
	if (_secure)
		result += "; Secure";
	if (_httpOnly)
		result += "; HttpOnly";
	return result;
}

} // !session
} // !webserv
