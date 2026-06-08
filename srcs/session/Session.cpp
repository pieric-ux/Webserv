/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Session.cpp
 * @brief Implements the Session class: an in-memory client session identified
 *        by a UUID, storing string key/value data with an expiry time and TTL.
 */

#include <webserv/session/Session.hpp>

namespace webserv
{
namespace session
{

/**
 * @brief Constructs a Session with the given identifier, empty data, and no
 *        expiry or TTL, then acquires its logger.
 *
 * @param id The UUID uniquely identifying this session.
 */
Session::Session(const t_uuid &id)
	:	_id(id),
		_data(),
		_expires(0),
		_ttl(0)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.session.session");
    _logger->setLevel(log42::logRecord::DEBUG);
    INFO(_logger, "Session instance created");
}

/**
 * @brief Destroys the session, releasing its held resources.
 */
Session::~Session() {}

/**
 * @brief Copy-constructs a session by duplicating the logger, id, data,
 *        expiry, and TTL of another session.
 *
 * @param rhs The session to copy from.
 */
Session::Session(const Session &rhs)
	:	_logger(rhs._logger),
		_id(rhs._id),
		_data(rhs._data),
		_expires(rhs._expires),
		_ttl(rhs._ttl)
{}

/**
 * @brief Copy-assigns the logger, id, data, expiry, and TTL from another
 *        session, guarding against self-assignment.
 *
 * @param rhs The session to assign from.
 * @return Reference to this session after assignment.
 */
Session &Session::operator=(const Session &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_id = rhs._id;
		_data = rhs._data;
		_expires = rhs._expires;
		_ttl = rhs._ttl;
	}
	return (*this);
}

/**
 * @brief Returns the shared logger for the session module, retrieved from the
 *        logging manager by its registered name.
 *
 * @return The logger associated with "webserv.session.session".
 */
t_Logger Session::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.session.session");
}

/**
 * @brief Returns the unique identifier of this session.
 *
 * @return Const reference to the session's UUID.
 */
const t_uuid &Session::getId() const
{
	return _id;
}

/**
 * @brief Returns the full key/value store held by this session.
 *
 * @return Const reference to the session's data map.
 */
const t_SessionData &Session::getData() const
{
	return _data;
}

/**
 * @brief Replaces the session's key/value store with the given data.
 *
 * @param data The new data map to store in the session.
 */
void Session::setData(const t_SessionData &data)
{
	_data = data;
}

/**
 * @brief Returns the absolute expiry time of this session.
 *
 * @return The expiry timestamp, or 0 if the session never expires.
 */
std::time_t Session::getExpires() const
{
	return _expires;
}

/**
 * @brief Sets the absolute expiry time of this session.
 *
 * @param expires The expiry timestamp; 0 marks the session as non-expiring.
 */
void Session::setExpires(std::time_t expires)
{
	_expires = expires;
}

t_sessionTTL Session::getTtl() const
{
	return _ttl;
}

void Session::setTtl(t_sessionTTL ttl)
{
	_ttl = ttl;
}

/**
 * @brief Reports whether the session has passed its expiry time.
 *
 * @return False if the expiry is unset (0), otherwise true when the current
 *         time is past the stored expiry timestamp.
 */
bool Session::isExpired() const
{
	if (_expires == 0)
		return false;
	return std::time(NULL) > _expires;
}

/**
 * @brief Looks up the value associated with a key in the session data.
 *
 * @param key The key to look up.
 * @return Const reference to the stored value, or to a static empty string if
 *         the key is absent.
 */
const std::string &Session::get(const std::string &key) const
{
	static const std::string empty;
	t_SessionData::const_iterator it = _data.find(key);
	if (it == _data.end())
		return empty;
	return it->second;
}

/**
 * @brief Stores or overwrites the value for a key in the session data.
 *
 * @param key The key to assign.
 * @param value The value to associate with the key.
 */
void Session::set(const std::string &key, const std::string &value)
{
	_data[key] = value;
}

} // !session
} // !webserv
