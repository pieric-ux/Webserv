/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Cookie.cpp
 * @brief Implements webserv::session::Cookie, an HTTP cookie storing a
 *        name/value pair plus its attributes (Expires, Path, Domain, Secure,
 *        HttpOnly) and serializing them into a Set-Cookie header value.
 */

#include <webserv/session/Cookie.hpp>

namespace webserv
{
namespace session
{

/**
 * @brief Constructs a cookie from a name/value pair with default attributes
 *        (no expiry, Path "/", no domain, not Secure, HttpOnly set) and
 *        acquires the session cookie logger.
 *
 * @param name Name of the cookie.
 * @param value Value associated with the cookie name.
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
 * @brief Destroys the cookie; holds no owned resources to release.
 */
Cookie::~Cookie() {}

/**
 * @brief Copy-constructs a cookie, duplicating the logger handle and all
 *        name, value and attribute fields.
 *
 * @param rhs Cookie to copy.
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
 * @brief Copy-assigns another cookie, copying the logger handle and all
 *        name, value and attribute fields while guarding against self-assignment.
 *
 * @param rhs Cookie to copy from.
 * @return Reference to this cookie after assignment.
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
 * @brief Returns the shared logger for the session cookie category.
 *
 * @return Logger handle for "webserv.session.cookie".
 */
t_Logger Cookie::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.session.cookie");
}

/**
 * @brief Returns the cookie name.
 *
 * @return Const reference to the cookie's name.
 */
const std::string &Cookie::getName() const
{
	return _name;
}

/**
 * @brief Returns the cookie value.
 *
 * @return Const reference to the cookie's value.
 */
const std::string &Cookie::getValue() const
{
	return _value;
}

/**
 * @brief Sets the cookie value.
 *
 * @param value New value to store for the cookie.
 */
void Cookie::setValue(const std::string &value)
{
	_value = value;
}

/**
 * @brief Returns the cookie expiry time.
 *
 * @return Expiry as a time_t; 0 means the cookie never expires.
 */
std::time_t Cookie::getExpires() const
{
	return _expires;
}

/**
 * @brief Sets the cookie expiry time.
 *
 * @param expires Expiry as a time_t; 0 means the cookie never expires.
 */
void Cookie::setExpires(std::time_t expires)
{
	_expires = expires;
}

/**
 * @brief Returns the cookie Path attribute.
 *
 * @return Const reference to the cookie's Path.
 */
const std::string &Cookie::getPath() const
{
	return _path;
}

/**
 * @brief Sets the cookie Path attribute.
 *
 * @param path New Path scope for the cookie.
 */
void Cookie::setPath(const std::string &path)
{
	_path = path;
}

/**
 * @brief Returns the cookie Domain attribute.
 *
 * @return Const reference to the cookie's Domain.
 */
const std::string &Cookie::getDomain() const
{
	return _domain;
}

/**
 * @brief Sets the cookie Domain attribute.
 *
 * @param domain New Domain scope for the cookie.
 */
void Cookie::setDomain(const std::string &domain)
{
	_domain = domain;
}

/**
 * @brief Reports whether the cookie carries the Secure attribute.
 *
 * @return true if the cookie is marked Secure, false otherwise.
 */
bool Cookie::isSecure() const
{
	return _secure;
}

/**
 * @brief Sets the cookie Secure attribute.
 *
 * @param secure true to mark the cookie Secure, false to clear it.
 */
void Cookie::setSecure(bool secure)
{
	_secure = secure;
}

/**
 * @brief Reports whether the cookie carries the HttpOnly attribute.
 *
 * @return true if the cookie is marked HttpOnly, false otherwise.
 */
bool Cookie::isHttpOnly() const
{
	return _httpOnly;
}

/**
 * @brief Sets the cookie HttpOnly attribute.
 *
 * @param httpOnly true to mark the cookie HttpOnly, false to clear it.
 */
void Cookie::setHttpOnly(bool httpOnly)
{
	_httpOnly = httpOnly;
}

/**
 * @brief Determines whether the cookie has expired relative to the current time.
 *
 * @return false if no expiry is set (expires == 0), otherwise true when the
 *         current time is past the expiry instant.
 */
bool Cookie::isExpired() const
{
	if (_expires == 0)
		return false;
	return std::time(NULL) > _expires;
}

/**
 * @brief Serializes the cookie into a Set-Cookie header value, emitting the
 *        name=value pair followed by any set attributes (Path, Domain, an
 *        Expires date formatted in GMT, Secure, HttpOnly).
 *
 * @return The assembled Set-Cookie value string.
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
