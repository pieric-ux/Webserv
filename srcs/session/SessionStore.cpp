/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionStore.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file SessionStore.cpp
 * @brief Implements the SessionStore singleton, which generates UUIDs and
 *        creates, retrieves, deletes and purges in-memory HTTP sessions.
 */

#include <cstdio>
#include <webserv/session/SessionStore.hpp>
#include <webserv/config/DefaultConfig.hpp>

namespace webserv
{
namespace session
{

/**
 * @brief Constructs the SessionStore, acquiring its logger and setting it to
 *        DEBUG level.
 */
SessionStore::SessionStore()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.session.sessionstore");
    _logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "SessionStore instance created");
}

/**
 * @brief Destroys the SessionStore; the session map releases its entries
 *        automatically.
 */
SessionStore::~SessionStore() {}

/**
 * @brief Accesses the unique SessionStore instance.
 *
 * @return Reference to the lazily-initialized singleton SessionStore.
 */
SessionStore &SessionStore::getInstance()
{
	static SessionStore instance;
	return instance;
}

/**
 * @brief Retrieves the logger associated with the session store module.
 *
 * @return The "webserv.session.sessionstore" logger.
 */
t_Logger SessionStore::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.session.sessionstore");
}

/**
 * @brief Generates a random RFC 4122 version 4 UUID using /dev/urandom.
 *
 * @return The UUID as a 36-character hyphenated lowercase hexadecimal string.
 */
t_uuid SessionStore::generateUUID()
{
	unsigned char bytes[16];

	std::FILE *f = std::fopen("/dev/urandom", "rb");
	if (!f)
		throw std::runtime_error("generateUUID: failed to open /dev/urandom");
	std::fread(bytes, 1, sizeof(bytes), f);
	std::fclose(f);

	bytes[6] = (bytes[6] & 0x0F) | 0x40;
	bytes[8] = (bytes[8] & 0x3F) | 0x80;

	char buf[37];
	std::snprintf(buf, sizeof(buf),
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		bytes[0],  bytes[1],  bytes[2],  bytes[3],
		bytes[4],  bytes[5],  bytes[6],  bytes[7],
		bytes[8],  bytes[9],  bytes[10], bytes[11],
		bytes[12], bytes[13], bytes[14], bytes[15]);
	return std::string(buf);
}

/**
 * @brief Creates a new session with a freshly generated UUID, applies the given
 *        TTL to compute its expiry time, and stores it in the session map.
 *
 * @return A copy of the newly created Session.
 */
Session SessionStore::createSession(t_sessionTTL ttl)
{
	t_uuid id = generateUUID();
	Session session(id);
	session.setTtl(ttl);
	session.setExpires(std::time(NULL) + static_cast<std::time_t>(ttl));
	_sessions.insert(std::make_pair(id, session));
	INFO(_logger, "Session created: " + id + " expires in " + common::core::utils::toString(ttl) + "s");
	return session;
}

/**
 * @brief Looks up a session by id, refreshing its expiry on access; throws if
 *        the session is missing or has expired (purging it in the latter case).
 *
 * @param id The UUID of the session to retrieve.
 * @return Reference to the live, non-expired Session.
 */
Session &SessionStore::getSession(const t_uuid &id)
{
	t_SessionStore::iterator it = _sessions.find(id);
	if (it == _sessions.end())
		throw std::runtime_error("Session not found: " + id);

	if (it->second.isExpired())
	{
		INFO(_logger, "Session expired, purging: " + id);
		_sessions.erase(it);
		throw std::runtime_error("Session expired: " + id);
	}

	if (it->second.getTtl() > 0)
		it->second.setExpires(std::time(NULL) + static_cast<std::time_t>(it->second.getTtl()));
	return it->second;
}

/**
 * @brief Removes the session with the given id from the store if it exists.
 *
 * @param id The UUID of the session to delete.
 */
void SessionStore::deleteSession(const t_uuid &id)
{
	t_SessionStore::iterator it = _sessions.find(id);
	if (it != _sessions.end())
	{
		_sessions.erase(it);
		INFO(_logger, "Session deleted: " + id);
	}
}

/**
 * @brief Iterates over all stored sessions and removes those that have expired.
 */
void SessionStore::purgeExpired()
{
	t_SessionStore::iterator it = _sessions.begin();
	while (it != _sessions.end())
	{
		if (it->second.isExpired())
		{
			INFO(_logger, "Purging expired session: " + it->first);
			t_SessionStore::iterator toErase = it;
			++it;
			_sessions.erase(toErase);
		}
		else
			++it;
	}
}

} // !session
} // !webserv
