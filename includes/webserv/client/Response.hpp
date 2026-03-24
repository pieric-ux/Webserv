// TODO: don't forget header

#ifndef WEBSERV_CLIENT_RESPONSE_HPP
#define WEBSERV_CLIENT_RESPONSE_HPP

/**
 * @file Response.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <string>
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

		std::string							getHttpVersion() const;
		void								setHttpVersion(const std::string &httpVersion);
		StatusCode							getStatusCode() const;
		void								setStatusCode(const StatusCode &statusCode);
		std::vector<std::list<HTTPHeader>>	getHeaders() const;
		void								setHeaders(const std::vector<std::list<HTTPHeader>> &headers);
		std::vector<unsigned char>			getBody() const;
		void								setBody(const std::vector<unsigned char> &body);
		e_ResponseFlags						getFlags() const;
		void								setFlags(const e_ResponseFlags flags);


	private:
		std::string							_httpVersion;
		StatusCode							_statusCode;
		std::vector<HTTPHeader>				_headers;
		std::vector<unsigned char>			_body;
		e_ResponseFlags						_flags;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_RESPONSE_HPP
