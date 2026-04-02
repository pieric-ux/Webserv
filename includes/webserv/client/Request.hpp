// TODO: don't forget header

#ifndef WEBSERV_CLIENT_REQUEST_HPP
#define WEBSERV_CLIENT_REQUEST_HPP

/**
 * @file Request.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/config/method.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/headers/HTTPHeader.hpp>

namespace webserv
{
namespace client
{

enum e_RequestFlags
{
	E_REQ_REQUEST_LINE = 1 << 0,
	E_REQ_HEADERS_VALIDATED = 1 << 1,
	E_REQ_BODY_STARTED = 1 << 2
};

class Request
{
	public:
		Request();
		~Request();

		Request(const Request &rhs);
		Request &operator=(const Request &rhs);
	
		static t_Logger					getLogger();

		config::e_Method			getMethod() const;
		void						setMethod(const config::e_Method method);
		std::string					getRequestTarget() const;
		void						setRequestTarget(const std::string &requestTarget);
		std::string					getHttpVersion() const;
		void						setHttpVersion(const std::string &httpVersion);
		std::string					getAbsolutePath() const;
		void						setAbsolutePath(const std::string &absolutePath);
		t_Headers					getHeaders() const;
		void						setHeaders(const t_Headers &headers);
		std::vector<unsigned char>	getBody() const;
		void						setBody(const t_raw &body);
		e_RequestFlags				getFlags() const;
		void						setFlags(const e_RequestFlags flags);

	private:
		t_Logger					_logger;
		config::e_Method			_method;
		std::string					_requestTarget;
		std::string					_httpVersion;
		std::string					_absolutePath;
		t_Headers					_headers;
		t_raw						_body;
		e_RequestFlags				_flags;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_REQUEST_HPP
