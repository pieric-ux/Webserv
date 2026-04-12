// TODO: don't forget header

#ifndef WEBSERV_CLIENT_REQUEST_HPP
#define WEBSERV_CLIENT_REQUEST_HPP

/**
 * @file Request.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <common/common.hpp>
#include <webserv/types.hpp>
#include <webserv/config/method.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/headers/HTTPHeadersRegistry.hpp>

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

		config::e_Method				getMethod() const;
		void							setMethod(const config::e_Method method);
		std::string						getRequestTarget() const;
		void							setRequestTarget(const std::string &requestTarget);
		std::string						getHttpVersion() const;
		void							setHttpVersion(const std::string &httpVersion);
		std::string						getAbsolutePath() const;
		void							setAbsolutePath(const std::string &absolutePath);
		std::string						getAuthority() const;
		void							setAuthority(const std::string &authority);
		std::string						getPath() const;
		void							setPath(const std::string &path);
		std::string						getQuery() const;
		void							setQuery(const std::string &query);
		const t_Headers					&getHeaders() const;
		t_Headers						&getHeaders();
		void							setHeaders(const t_Headers &headers);
		const HTTPheaders::HTTPHeader	&findHeader(const std::string &headerName, const std::string &headerValue) const;
		void							addHeader(const std::string &headerName, const std::string &headerValue);
		int								getFlags() const;
		void							setFlags(const int flags);
		const config::LocationConfig	&getLocationConfig() const;
		void							setLocationConfig(const config::LocationConfig &locationConfig);

	private:
		t_Logger						_logger;
		config::e_Method				_method;
		config::LocationConfig			_locationConfig;
		std::string						_requestTarget;
		std::string						_httpVersion;
		std::string						_absolutePath;
		std::string 					_authority;
		std::string						_path;
		std::string						_query;
		t_Headers						_headers;
		e_RequestFlags					_flags;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_REQUEST_HPP
