// TODO: don't forget header

#ifndef WEBSERV_SESSION_SESSIONSTORE_HPP
#define WEBSERV_SESSION_SESSIONSTORE_HPP

/**
 * @file SessionStore.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/session/Session.hpp>

namespace webserv
{
namespace session
{

class SessionStore
{
	public:
		static SessionStore		&getInstance();
		static t_Logger			getLogger();

		Session					createSession(t_sessionTTL ttl);
		Session					&getSession(const t_uuid &id);
		void					deleteSession(const t_uuid &id);
		void					purgeExpired();

	private:
		SessionStore();
		~SessionStore();
		SessionStore(const SessionStore &rhs);
		SessionStore &operator=(const SessionStore &rhs);

		t_Logger				_logger;
		t_SessionStore			_sessions;

		static t_uuid			generateUUID();
};

} // !session
} // !webserv

#endif // !WEBSERV_SESSION_SESSIONSTORE_HPP
