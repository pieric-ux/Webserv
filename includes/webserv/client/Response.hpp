	// TODO: don't forget header

#ifndef WEBSERV_CLIENT_RESPONSE_HPP
#define WEBSERV_CLIENT_RESPONSE_HPP

/**
 * @file Response.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/headers/HTTPHeader.hpp>
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

		static t_Logger				getLogger();

		std::string				getHttpVersion() const;
		void					setHttpVersion(const std::string &httpVersion);
		status::StatusCode		getStatusCode() const;
		void					setStatusCode(const status::StatusCode &statusCode);
		t_Headers				getHeaders() const;
		void					setHeaders(const t_Headers &headers);
		t_raw					getBody() const;
		void					setBody(const t_raw &body);
		e_ResponseFlags			getFlags() const;
		void					setFlags(const e_ResponseFlags flags);

	private:
		t_Logger				_logger;
		std::string				_httpVersion;
		status::StatusCode		_statusCode;
		t_Headers				_headers;
		t_raw					_body;
		e_ResponseFlags			_flags;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_RESPONSE_HPP
