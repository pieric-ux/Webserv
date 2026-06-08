/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Request.cpp
 * @brief Implements the client::Request class, providing accessors and mutators for a parsed HTTP/1.1 request: method, request line components, resolved URI parts, headers, cookies, parsing-state flags, and the matched location configuration.
 */

#include <webserv/client/Request.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief Default-constructs an empty request with method GET, cleared URI and header fields, no flags set, and an initialized logger at DEBUG level.
 */
Request::Request()
	:	_method(config::GET),
		_locationConfig(),
		_requestTarget(),
		_httpVersion(),
		_absolutePath(),
		_authority(),
		_path(),
		_query(),
		_headers(),
		_flags(static_cast<e_RequestFlags>(0)),
		_cookies()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.request");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Request instance created");
}

/**
 * @brief Destroys the request; no owned resources require explicit release.
 */
Request::~Request() {}

/**
 * @brief Copy-constructs a request by duplicating all method, URI, header, cookie, flag, and location-configuration members of another request.
 *
 * @param rhs The request whose state is copied.
 */
Request::Request(const Request &rhs)
	:	_method(rhs._method),
		_locationConfig(rhs._locationConfig),
		_requestTarget(rhs._requestTarget),
		_httpVersion(rhs._httpVersion),
		_absolutePath(rhs._absolutePath),
		_authority(rhs._authority),
		_path(rhs._path),
		_query(rhs._query),
		_headers(rhs._headers),
		_flags(rhs._flags),
		_cookies(rhs._cookies)
{}

/**
 * @brief Copy-assigns the state of another request, with self-assignment protection.
 *
 * @param rhs The request whose state is copied.
 * @return Reference to this request after assignment.
 */
Request &Request::operator=(const Request &rhs)
{
	if (this != &rhs)
	{
		_method = rhs._method;
		_requestTarget = rhs._requestTarget;
		_httpVersion = rhs._httpVersion;
		_absolutePath = rhs._absolutePath;
		_authority = rhs._authority;
		_path = rhs._path;
		_query = rhs._query;
		_headers = rhs._headers;
		_locationConfig = rhs._locationConfig;
		_flags = rhs._flags;
		_cookies = rhs._cookies;
	}
	return (*this);
}

/**
 * @brief Retrieves the shared logger registered for the request component.
 *
 * @return Shared pointer to the "webserv.client.request" logger.
 */
t_Logger	Request::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.request");
}

/**
 * @brief Returns the HTTP method of the request.
 *
 * @return The request method.
 */
config::e_Method Request::getMethod() const
{
	return _method;
}

/**
 * @brief Sets the HTTP method of the request.
 *
 * @param method The method to store.
 */
void Request::setMethod(const config::e_Method method)
{
	_method = method;
}

/**
 * @brief Returns the raw request target from the request line.
 *
 * @return The request target as received.
 */
std::string Request::getRequestTarget() const
{
	return _requestTarget;
}

/**
 * @brief Sets the raw request target of the request line.
 *
 * @param requestTarget The request target to store.
 */
void Request::setRequestTarget(const std::string &requestTarget)
{
	_requestTarget = requestTarget;
}

/**
 * @brief Returns the HTTP version string from the request line.
 *
 * @return The HTTP version (e.g. "HTTP/1.1").
 */
std::string Request::getHttpVersion() const
{
	return _httpVersion;
}

/**
 * @brief Sets the HTTP version string of the request line.
 *
 * @param httpVersion The HTTP version to store.
 */
void Request::setHttpVersion(const std::string &httpVersion)
{
	_httpVersion = httpVersion;
}

/**
 * @brief Returns the resolved absolute filesystem path for the request target.
 *
 * @return The absolute path.
 */
std::string Request::getAbsolutePath() const
{
	return _absolutePath;
}

/**
 * @brief Sets the resolved absolute filesystem path for the request target.
 *
 * @param absolutePath The absolute path to store.
 */
void Request::setAbsolutePath(const std::string &absolutePath)
{
	_absolutePath = absolutePath;
}

/**
 * @brief Returns the authority component (host and optional port) of the request URI.
 *
 * @return The authority string.
 */
std::string Request::getAuthority() const
{
	return _authority;
}

/**
 * @brief Sets the authority component (host and optional port) of the request URI.
 *
 * @param authority The authority string to store.
 */
void Request::setAuthority(const std::string &authority)
{
	_authority = authority;
}

/**
 * @brief Returns the path component of the request URI (without the query string).
 *
 * @return The URI path.
 */
std::string Request::getPath() const
{
	return _path;
}

/**
 * @brief Sets the path component of the request URI.
 *
 * @param path The URI path to store.
 */
void Request::setPath(const std::string &path)
{
	_path = path;
}

/**
 * @brief Returns the query-string component of the request URI.
 *
 * @return The query string (without the leading '?').
 */
std::string Request::getQuery() const
{
	return _query;
}

/**
 * @brief Sets the query-string component of the request URI.
 *
 * @param query The query string to store.
 */
void Request::setQuery(const std::string &query)
{
	_query = query;
}

/**
 * @brief Returns a read-only reference to the request headers, keyed by lowercased name.
 *
 * @return Const reference to the header map.
 */
const t_Headers &Request::getHeaders() const
{
	return _headers;
}

t_Headers &Request::getHeaders()
{
	return _headers;
}
/**
 * @brief Replaces the request headers with the provided header map.
 *
 * @param headers The header map to store.
 */
void Request::setHeaders(const t_Headers &headers)
{
	_headers = headers;
}

/**
 * @brief Looks up a header by name (case-insensitive) and exact value among its possibly multiple entries.
 *
 * @param headerName The header field name to search for.
 * @param headerValue The exact value the matched header must hold.
 * @return Const reference to the matching header, or a reference to a static empty header if none matches.
 */
const HTTPheaders::HTTPHeader &Request::findHeader(const std::string &headerName, const std::string &headerValue) const
{
	static HTTPheaders::HTTPHeader emptyHeader;

	t_Headers::const_iterator it = _headers.find(common::core::utils::toLower(headerName));
	if (it == _headers.end())
		return emptyHeader;

	const std::list<HTTPheaders::HTTPHeader> &headerList = it->second;
	std::list<HTTPheaders::HTTPHeader>::const_iterator lit = headerList.begin();
	for (;lit != headerList.end(); ++lit)
	{
		if (lit->getValue() == headerValue)
			return *lit;
	}
	return emptyHeader;
}

/**
 * @brief Appends a header by resolving its definition from the headers registry, assigning the value, and storing it under the lowercased name.
 *
 * @param headerName The header field name.
 * @param headerValue The value to assign to the header.
 */
void Request::addHeader(const std::string &headerName, const std::string &headerValue)
{
	HTTPheaders::HTTPHeader header = HTTPheaders::HTTPHeadersRegistry::getInstance().getHeader(headerName);
	header.setValue(headerValue);
	_headers[common::core::utils::toLower(headerName)].push_back(header);
}

/**
 * @brief Returns the current parsing-state flags as a bitmask of e_RequestFlags values.
 *
 * @return The combined request flags.
 */
int	Request::getFlags() const
{
	return _flags;
}

/**
 * @brief Sets the parsing-state flags from an integer bitmask of e_RequestFlags values.
 *
 * @param flags The bitmask to store as the request flags.
 */
void	Request::setFlags(const int flags)
{
	_flags = static_cast<e_RequestFlags>(flags);
}

const config::LocationConfig &Request::getLocationConfig() const
{
	return _locationConfig;
}

void Request::setLocationConfig(const config::LocationConfig &locationConfig)
{
	_locationConfig = locationConfig;
}

/**
 * @brief Returns a read-only reference to the request cookies, keyed by cookie name.
 *
 * @return Const reference to the cookie map.
 */
const t_Cookies &Request::getCookies() const
{
	return _cookies;
}

/**
 * @brief Replaces the request cookies with the provided cookie map.
 *
 * @param cookies The cookie map to store.
 */
void Request::setCookies(const t_Cookies &cookies)
{
	_cookies = cookies;
}

} // !client
} // !webserv
