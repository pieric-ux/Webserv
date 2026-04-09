// TODO: don't forget header

/**
 * @file Response.cpp
 * @brief [TODO:description]
 */

#include <webserv/client/Response.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief [TODO:description]
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
 * @brief [TODO:description]
 */
Response::~Response() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Response::Response(const Response &rhs)
	:	_httpVersion(rhs._httpVersion),
		_statusCode(rhs._statusCode),
		_headers(rhs._headers),
		_flags(rhs._flags)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
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
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	Response::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.response");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string Response::getHttpVersion() const
{
	return _httpVersion;
}

/**
 * @brief [TODO:description]
 *
 * @param httpVersion [TODO:parameter]
 */
void Response::setHttpVersion(const std::string &httpVersion)
{
	_httpVersion = httpVersion;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
status::StatusCode Response::getStatusCode() const
{
	return _statusCode;
}

/**
 * @brief [TODO:description]
 *
 * @param statusCode [TODO:parameter]
 */
void Response::setStatusCode(const status::StatusCode &statusCode)
{
	_statusCode = statusCode;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Headers Response::getHeaders() const
{
	return _headers;
}

/**
 * @brief [TODO:description]
 *
 * @param headers [TODO:parameter]
 */
void Response::setHeaders(const t_Headers &headers)
{
	_headers = headers;
}

/**
 * @brief [TODO:description]
 *
 * @param headerName [TODO:parameter]
 * @param headerValue [TODO:parameter]
 * @return [TODO:return]
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
 * @brief [TODO:description]
 *
 * @param headerName [TODO:parameter]
 * @param headerValue [TODO:parameter]
 */
void	Response::addHeader(const std::string &headerName, const std::string &headerValue)
{
	HTTPheaders::HTTPHeader header = HTTPheaders::HTTPHeadersRegistry::getInstance().getHeader(headerName);
	header.setValue(headerValue);
	_headers[common::core::utils::toLower(headerName)].push_back(header);
}


/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
e_ResponseFlags Response::getFlags() const
{
	return _flags;
}

/**
 * @brief [TODO:description]
 *
 * @param flags [TODO:parameter]
 */
void Response::setFlags(const e_ResponseFlags flags)
{
	_flags = flags;
}

} // !client
} // !webserv
