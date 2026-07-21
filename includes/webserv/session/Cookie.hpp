/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SESSION_COOKIE_HPP
#define WEBSERV_SESSION_COOKIE_HPP

/**
 * @file Cookie.hpp
 * @brief Declares webserv::session::Cookie, an HTTP cookie holding a
 *        name/value pair and its attributes (Expires, Path, Domain, Secure,
 *        HttpOnly) and able to serialize itself into a Set-Cookie value.
 */

#include <ctime>
#include <string>
#include <webserv/types.hpp>

namespace webserv
{
namespace session
{

class Cookie
{
	public:
		Cookie(const std::string &name, const std::string &value);
		~Cookie();

		Cookie(const Cookie &rhs);
		Cookie &operator=(const Cookie &rhs);

		static t_Logger			getLogger();

		const std::string		&getName() const;
		const std::string		&getValue() const;
		void					setValue(const std::string &value);
		std::time_t				getExpires() const;
		void					setExpires(std::time_t expires);
		const std::string		&getPath() const;
		void					setPath(const std::string &path);
		const std::string		&getDomain() const;
		void					setDomain(const std::string &domain);
		bool					isSecure() const;
		void					setSecure(bool secure);
		bool					isHttpOnly() const;
		void					setHttpOnly(bool httpOnly);
		bool					isExpired() const;
		std::string				serializeCookie() const;

	private:
		t_Logger				_logger;
		std::string				_name;
		std::string				_value;
		std::time_t				_expires;
		std::string				_path;
		std::string				_domain;
		bool					_secure;
		bool					_httpOnly;
};

} // !session
} // !webserv

#endif // !WEBSERV_SESSION_COOKIE_HPP
