/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CLIENT_HTTPERROR_HPP
#define WEBSERV_CLIENT_HTTPERROR_HPP

/**
 * @file HTTPError.hpp
 * @brief Exception type representing an HTTP error response, carrying a status code and optional redirect location.
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/status/StatusCode.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>

namespace webserv
{
namespace client
{

class HTTPError : public std::exception
{
	public:
		HTTPError();
		explicit HTTPError(unsigned short code);
		HTTPError(unsigned short code, const std::string &location);
		virtual ~HTTPError() throw();

		HTTPError(const HTTPError &rhs);
		HTTPError &operator=(const HTTPError &rhs);

		static t_Logger			getLogger();

		status::StatusCode		getStatusCode() const;
		void					setStatusCode(const status::StatusCode &statusCode);
		const std::string		&getLocation() const;
		const char				*what() const throw();

	private:
		t_Logger				_logger;
		status::StatusCode		_statusCode;
		std::string				_what;
		std::string				_location;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_HTTPERROR_HPP
