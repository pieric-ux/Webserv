// TODO: don't forget header

#ifndef WEBSERV_SESSION_SESSION_HPP
#define WEBSERV_SESSION_SESSION_HPP

/**
 * @file Session.hpp
 * @brief [TODO:description]
 */

#include <ctime>
#include <string>
#include <webserv/types.hpp>

namespace webserv
{
namespace session
{

class Session
{
	public:
		Session(const t_uuid &id);
		~Session();

		Session(const Session &rhs);
		Session &operator=(const Session &rhs);

		static t_Logger							getLogger();

		const t_uuid							&getId() const;
		const t_SessionData						&getData() const;
		void									setData(const t_SessionData &data);
		std::time_t								getExpires() const;
		void									setExpires(std::time_t expires);
		t_sessionTTL							getTtl() const;
		void									setTtl(t_sessionTTL ttl);
		bool									isExpired() const;
		const std::string						&get(const std::string &key) const;
		void									set(const std::string &key, const std::string &value);

	private:
		t_Logger								_logger;
		t_uuid									_id;
		t_SessionData							_data;
		std::time_t								_expires;
		t_sessionTTL							_ttl;
};

} // !session
} // !webserv

#endif // !WEBSERV_SESSION_SESSION_HPP
