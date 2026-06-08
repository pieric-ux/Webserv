/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Response.cpp
 * @brief Implements the client::Response HTTP response model, providing accessors
 *        for the protocol version, status code, headers, flags, and the
 *        connection-close indicator.
 */

#include <webserv/client/Response.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief Constructs an empty response with zero-initialized version, status code,
 *        headers, and flags, and acquires the response logger.
 */
Response::Response()
	:	_httpVersion(),
		_statusCode(),
		_headers(),
		_flags(static_cast<e_ResponseFlags>(0))
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.response");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Response instance created");
}

/**
 * @brief Destroys the response; no manual resource cleanup is required.
 */
Response::~Response() {}

/**
 * @brief Copy-constructs a response by copying the version, status code, headers,
 *        and flags from another instance.
 *
 * @param rhs The response to copy from.
 */
Response::Response(const Response &rhs)
	:	_httpVersion(rhs._httpVersion),
		_statusCode(rhs._statusCode),
		_headers(rhs._headers),
		_flags(rhs._flags)
{}

/**
 * @brief Copy-assigns the version, status code, headers, and flags from another
 *        response, guarding against self-assignment.
 *
 * @param rhs The response to copy from.
 * @return Reference to this response.
 */
Response &Response::operator=(const Response &rhs)
{
	if (this != &rhs)
	{
		_httpVersion = rhs._httpVersion;
		_statusCode = rhs._statusCode;
		_headers = rhs._headers;
		_flags = rhs._flags;
	}
	return (*this);
}

/**
 * @brief Returns the shared logger used by the response module.
 *
 * @return The "webserv.client.response" logger instance.
 */
t_Logger	Response::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.response");
}

/**
 * @brief Returns the HTTP protocol version of the response.
 *
 * @return The response's HTTP version string.
 */
std::string Response::getHttpVersion() const
{
	return _httpVersion;
}

/**
 * @brief Sets the HTTP protocol version of the response.
 *
 * @param httpVersion The HTTP version string to assign.
 */
void Response::setHttpVersion(const std::string &httpVersion)
{
	_httpVersion = httpVersion;
}

/**
 * @brief Returns the HTTP status code of the response.
 *
 * @return The response's status code.
 */
status::StatusCode Response::getStatusCode() const
{
	return _statusCode;
}

/**
 * @brief Sets the HTTP status code of the response.
 *
 * @param statusCode The status code to assign.
 */
void Response::setStatusCode(const status::StatusCode &statusCode)
{
	_statusCode = statusCode;
}

/**
 * @brief Returns the response's header collection for read-only access.
 *
 * @return Const reference to the map of lowercased header names to their values.
 */
const t_Headers &Response::getHeaders() const
{
	return _headers;
}

t_Headers &Response::getHeaders()
{
	return _headers;
}
/**
 * @brief Replaces the response's header collection.
 *
 * @param headers The header map to assign.
 */
void Response::setHeaders(const t_Headers &headers)
{
	_headers = headers;
}

/**
 * @brief Looks up a header by name (case-insensitive) and exact value among its
 *        stored entries.
 *
 * @param headerName The header field name to search for.
 * @param headerValue The exact value to match within that header's entries.
 * @return Reference to the matching header, or a static empty header if no match
 *         is found.
 */
const HTTPheaders::HTTPHeader &Response::findHeader(const std::string &headerName, const std::string &headerValue) const
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
 * @brief Adds a header by resolving its definition from the headers registry,
 *        setting the given value, and appending it under the lowercased name.
 *
 * @param headerName The header field name to add.
 * @param headerValue The value to assign to the new header entry.
 */
void	Response::addHeader(const std::string &headerName, const std::string &headerValue)
{
	HTTPheaders::HTTPHeader header = HTTPheaders::HTTPHeadersRegistry::getInstance().getHeader(headerName);
	header.setValue(headerValue);
	_headers[common::core::utils::toLower(headerName)].push_back(header);
}


/**
 * @brief Returns the response's processing flags.
 *
 * @return The current flags as an integer bitmask (see e_ResponseFlags).
 */
int	Response::getFlags() const
{
	return _flags;
}

/**
 * @brief Sets the response's processing flags from an integer bitmask.
 *
 * @param flags The flag bitmask to assign (see e_ResponseFlags).
 */
void	Response::setFlags(const int flags)
{
	_flags = static_cast<e_ResponseFlags>(flags);
}

/**
 * @brief Indicates whether the connection should be closed after this response.
 *
 * @return True if the connection should be closed, false otherwise.
 */
bool Response::shouldCloseConnection() const
{
	return _shouldCloseConnection;
}

/**
 * @brief Sets whether the connection should be closed after this response.
 *
 * @param shouldClose True to close the connection after sending, false to keep it
 *        alive.
 */
void Response::setShouldCloseConnection(bool shouldClose)
{
	_shouldCloseConnection = shouldClose;
}

} // !client
} // !webserv
