/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CLIENT_RESPONSE_HPP
#define WEBSERV_CLIENT_RESPONSE_HPP

/**
 * @file Response.hpp
 * @brief Declares the client::Response class, an HTTP response model holding the
 *        protocol version, status code, headers, processing flags, and the
 *        connection-close indicator.
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/headers/HTTPHeadersRegistry.hpp>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace client
{

enum e_ResponseFlags
{
	E_RESP_HEADERS_SENT = 1 << 0
};

class Response
{
	public:
		Response();
		~Response();

		Response(const Response &rhs);
		Response &operator=(const Response &rhs);

		static t_Logger					getLogger();

		std::string						getHttpVersion() const;
		void							setHttpVersion(const std::string &httpVersion);
		status::StatusCode				getStatusCode() const;
		void							setStatusCode(const status::StatusCode &statusCode);
		const t_Headers					&getHeaders() const;
		t_Headers						&getHeaders();
		void							setHeaders(const t_Headers &headers);
		const HTTPheaders::HTTPHeader	&findHeader(const std::string &headerName, const std::string &headerValue) const;
		void							addHeader(const std::string &headerName, const std::string &headerValue);
		int								getFlags() const;
		void							setFlags(const int flags);
		bool							shouldCloseConnection() const;
		void							setShouldCloseConnection(bool shouldClose);

	private:
		t_Logger						_logger;
		std::string						_httpVersion;
		status::StatusCode				_statusCode;
		t_Headers						_headers;
		e_ResponseFlags					_flags;
		bool							_shouldCloseConnection;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_RESPONSE_HPP
