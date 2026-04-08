// TODO: don't forget header

/**
 * @file Request.cpp
 * @brief [TODO:description]
 */

#include <webserv/client/Request.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief [TODO:description]
 */
Request::Request()
	:	_method(config::GET),
		_requestTarget(),
		_httpVersion(),
		_absolutePath(),
		_headers(),
		_body(),
		_flags(static_cast<e_RequestFlags>(0))
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.request");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Request instance created");
}

/**
 * @brief [TODO:description]
 */
Request::~Request() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Request::Request(const Request &rhs)
	:	_method(rhs._method),
		_requestTarget(rhs._requestTarget),
		_httpVersion(rhs._httpVersion),
		_absolutePath(rhs._absolutePath),
		_headers(rhs._headers),
		_body(rhs._body),
		_flags(rhs._flags)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
Request &Request::operator=(const Request &rhs)
{
	if (this != &rhs)
	{
		_method = rhs._method;
		_requestTarget = rhs._requestTarget;
		_httpVersion = rhs._httpVersion;
		_absolutePath = rhs._absolutePath;
		_headers = rhs._headers;
		_body = rhs._body;
		_flags = rhs._flags;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	Request::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.request");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
config::e_Method Request::getMethod() const
{
	return _method;
}

/**
 * @brief [TODO:description]
 *
 * @param method [TODO:parameter]
 */
void Request::setMethod(const config::e_Method method)
{
	_method = method;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string Request::getRequestTarget() const
{
	return _requestTarget;
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 */
void Request::setRequestTarget(const std::string &requestTarget)
{
	_requestTarget = requestTarget;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string Request::getHttpVersion() const
{
	return _httpVersion;
}

/**
 * @brief [TODO:description]
 *
 * @param httpVersion [TODO:parameter]
 */
void Request::setHttpVersion(const std::string &httpVersion)
{
	_httpVersion = httpVersion;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string Request::getAbsolutePath() const
{
	return _absolutePath;
}

/**
 * @brief [TODO:description]
 *
 * @param absolutePath [TODO:parameter]
 */
void Request::setAbsolutePath(const std::string &absolutePath)
{
	_absolutePath = absolutePath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Headers Request::getHeaders() const
{
	return _headers;
}

/**
 * @brief [TODO:description]
 *
 * @param headers [TODO:parameter]
 */
void Request::setHeaders(const t_Headers &headers)
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
 * @brief [TODO:description]
 *
 * @param headerName [TODO:parameter]
 * @param headerValue [TODO:parameter]
 */
void Request::addHeader(const std::string &headerName, const std::string &headerValue)
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
t_raw	Request::getBody() const
{
	return _body;
}

/**
 * @brief [TODO:description]
 *
 * @param body [TODO:parameter]
 */
void Request::setBody(const t_raw &body)
{
	_body = body;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
e_RequestFlags Request::getFlags() const
{
	return _flags;
}

/**
 * @brief [TODO:description]
 *
 * @param flags [TODO:parameter]
 */
void Request::setFlags(const e_RequestFlags flags)
{
	_flags = flags;
}

} // !client
} // !webserv
