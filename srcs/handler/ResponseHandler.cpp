/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file ResponseHandler.cpp
 * @brief Implements ResponseHandler, which builds HTTP responses by assembling
 *        the status line, headers, cookies and error bodies into an outgoing
 *        byte buffer.
 */

#include <webserv/handler/ResponseHandler.hpp>
#include <webserv/handler/ExecutionHandler.hpp>
#include <webserv/session/SessionStore.hpp>
#include <webserv/session/Cookie.hpp>

namespace webserv
{
namespace handler
{

/**
 * @brief Constructs a ResponseHandler with empty response and buffer, and
 *        initializes its logger at DEBUG level.
 */
ResponseHandler::ResponseHandler()
	:	_response(),
		_bufferResponse()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.responsehandler");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ResponseHandler instance created");
}

/**
 * @brief Destroys the ResponseHandler.
 */
ResponseHandler::~ResponseHandler() {}

/**
 * @brief Copy-constructs a ResponseHandler from another instance.
 *
 * @param rhs The ResponseHandler whose logger, response and buffer are copied.
 */
ResponseHandler::ResponseHandler(const ResponseHandler &rhs)
	:	_logger(rhs._logger),
		_response(rhs._response),
		_bufferResponse(rhs._bufferResponse)
{}

/**
 * @brief Copy-assigns from another ResponseHandler.
 *
 * @param rhs The ResponseHandler whose logger, response and buffer are copied.
 * @return Reference to this ResponseHandler.
 */
ResponseHandler &ResponseHandler::operator=(const ResponseHandler &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_response = rhs._response;
		_bufferResponse = rhs._bufferResponse;
	}
	return (*this);
}

/**
 * @brief Retrieves the logger associated with this handler.
 *
 * @return The "webserv.handler.responsehandler" logger instance.
 */
t_Logger	ResponseHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.responsehandler");
}

/**
 * @brief Provides mutable access to the underlying response object.
 *
 * @return Reference to the response being built.
 */
client::Response &ResponseHandler::getResponse()
{
	return _response;
}

/**
 * @brief Provides read-only access to the underlying response object.
 *
 * @return Const reference to the response being built.
 */
const client::Response &ResponseHandler::getResponse() const
{
	return _response;
}

/**
 * @brief Replaces the underlying response object.
 *
 * @param response The response to copy into this handler.
 */
void ResponseHandler::setResponse(const client::Response &response)
{
	_response = response;
}

/**
 * @brief Appends raw bytes to the end of the outgoing response buffer.
 *
 * @param buffer The bytes to append to the response buffer.
 */
void ResponseHandler::appendToBufferResponse(const t_raw &buffer)
{
	_bufferResponse.insert(_bufferResponse.end(), buffer.begin(), buffer.end());
}

/**
 * @brief Empties the outgoing response buffer.
 */
void ResponseHandler::clearBufferResponse()
{
	_bufferResponse.clear();
}

/**
 * @brief Returns a const reference to the outgoing response buffer.
 */
const t_raw &ResponseHandler::getBufferResponse() const
{
	return _bufferResponse;
}

/**
 * @brief Erases the first @p n bytes from the outgoing response buffer.
 *
 * @param n Number of bytes to drop from the front of the buffer.
 */
void ResponseHandler::eraseBufferResponseFront(std::size_t n)
{
	if (n >= _bufferResponse.size())
		_bufferResponse.clear();
	else
		_bufferResponse.erase(_bufferResponse.begin(), _bufferResponse.begin() + n);
}

/**
 * @brief Builds a successful response: selects the status code from the
 *        execution and parse flags, writes the status line, assembles the
 *        headers and serializes them into the response buffer (skipping the
 *        header block for 100-Continue responses).
 *
 * @param request The request being responded to, used for HTTP version and headers.
 * @param execFlags Execution-result bitmask selecting the success status code
 *        (E_EXEC_CREATED -> 201, E_EXEC_NOCONTENT -> 204).
 * @param parseFlags Parser bitmask; E_PARS_EXPECT selects 100-Continue and
 *        suppresses serialization of the header block.
 */
void ResponseHandler::buildHeadersResponse(const client::Request &request, int execFlags, int parseFlags)
{
	DEBUG(_logger, "execFlags=0x" + common::core::utils::toString(execFlags)
		+ " parsFlags=0x" + common::core::utils::toString(parseFlags));

	// set status codes
	if (execFlags & E_EXEC_CREATED)
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(201));
	else if (execFlags & E_EXEC_NOCONTENT)
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(204));
	else if (parseFlags & parser::E_PARS_EXPECT)
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(100));
	else
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(200));
	DEBUG(_logger, "status code set to " + common::core::utils::toString(_response.getStatusCode().getCode()));
	
	buildStatusLine(request.getHttpVersion(), _response.getStatusCode(), _response.getStatusCode().getMessage());
	buildHeaders(request, parseFlags);

	int code = _response.getStatusCode().getCode();
	if (code == 201)
		_response.addHeader("Content-Length", "0");

	//append headers to bufferReponse
	if (!(parseFlags & parser::E_PARS_EXPECT))
	{
		const t_Headers &headers = _response.getHeaders();
		for (t_Headers::const_reverse_iterator rit = headers.rbegin(); rit != headers.rend(); ++rit)
		{
			const std::list<HTTPheaders::HTTPHeader> &headerList = rit->second;
			for (std::list<HTTPheaders::HTTPHeader>::const_iterator lit = headerList.begin(); lit != headerList.end(); ++lit)
			{
				std::string headerLine = lit->getName() + ": " + lit->getValue() + "\r\n";
				appendToBufferResponse(t_raw(headerLine.begin(), headerLine.end()));
				DEBUG(_logger, "header added to buffer: " + headerLine);
			}
		}
	}

	unsigned char crlf[] = {'\r', '\n'};
	appendToBufferResponse(t_raw(crlf, crlf + 2));
	DEBUG(_logger, "headers serialized, buffer size=" + common::core::utils::toString(_bufferResponse.size()));
}

/**
 * @brief Builds an error response: sets the error status code, writes the
 *        status line and status-specific headers (Allow, WWW-Authenticate,
 *        Retry-After, Accept, Upgrade, Location), loads a configured custom
 *        error page or generates a default HTML body, then serializes the
 *        headers and body into the response buffer.
 *
 * @param request The originating request, used for HTTP version and location config.
 * @param error The HTTP error carrying the status code and optional Location target.
 */
void ResponseHandler::buildErrorResponse(const client::Request &request, const client::HTTPError &error)
{
	DEBUG(_logger, "building error response for " + common::core::utils::toString(error.getStatusCode().getCode()) + " " + error.getStatusCode().getMessage());

	_response.setStatusCode(error.getStatusCode());
	DEBUG(_logger, "status code set to " + common::core::utils::toString(_response.getStatusCode().getCode()));

	std::string httpVersion = request.getHttpVersion();
	if (httpVersion.empty())
		httpVersion = "HTTP/1.1";
	buildStatusLine(httpVersion, _response.getStatusCode(), _response.getStatusCode().getMessage());
	
	_response.addHeader("Server", "webserv/1.0");

	std::time_t t = std::time(NULL);
	std::tm tm = *std::gmtime(&t);
	char dateStr[100];
	std::strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	_response.addHeader("Date", dateStr);
	if (!error.getLocation().empty())
		_response.addHeader("Location", error.getLocation());

	if (error.getStatusCode().getCode() == 401)
		_response.addHeader("WWW-Authenticate", "Basic realm=\"Restricted Area\"");

	if (error.getStatusCode().getCode() == 405)
	{
		t_AllowedMethods allowMethods = request.getLocationConfig().getAllowedMethods();
		t_AllowedMethods::const_iterator it = allowMethods.begin();
		std::string allowHeaderValue;
		for (; it != allowMethods.end(); ++it)
		{
			switch (*it)
			{
				case config::GET:
					allowHeaderValue += "GET, ";
					break;
				case config::HEAD:
					allowHeaderValue += "HEAD, ";
					break;
				case config::POST:
					allowHeaderValue += "POST, ";
					break;
				case config::PUT:
					allowHeaderValue += "PUT, ";
					break;
				case config::DELETE:
					allowHeaderValue += "DELETE, ";
					break;
				default:
					break;
			}
		}
		if (!allowHeaderValue.empty())
		{
			allowHeaderValue = allowHeaderValue.substr(0, allowHeaderValue.size() - 2);
			_response.addHeader("Allow", allowHeaderValue);
		}
	}

	if (error.getStatusCode().getCode() == 413 || error.getStatusCode().getCode() == 503)
		_response.addHeader("Retry-After", "42");

	if (error.getStatusCode().getCode() == 415)
	{
		std::string acceptedTypes;

		t_MimeTypes::const_iterator it = request.getLocationConfig().getTypes().begin();
		for (; it != request.getLocationConfig().getTypes().end(); ++it)
		{
			acceptedTypes += it->second + ", ";
		}
		if (!acceptedTypes.empty())
		{
			acceptedTypes = acceptedTypes.substr(0, acceptedTypes.size() - 2);
			_response.addHeader("Accept", acceptedTypes);
		}
	}

	if (error.getStatusCode().getCode() == 426)
		_response.addHeader("Upgrade", "HTTP/1.1");
	
	std::string errorBody = "";
	if (error.getStatusCode().getCode() == 304)
	{
		_response.addHeader("Content-Length", "1");
		errorBody = "";
		_response.addHeader("Content-Type", "application/octet-stream");
	}
	else
	{
		const t_ErrorPages &errorPages = request.getLocationConfig().getErrorPage();
		bool customPageFound = false;

		for (t_ErrorPages::const_iterator ep = errorPages.begin(); ep != errorPages.end(); ++ep)
		{
		    const t_StatusCodes &codes = ep->getCodes();
		    for (t_StatusCodes::const_iterator c = codes.begin(); c != codes.end(); ++c)
		    {
		        if (c->getCode() == error.getStatusCode().getCode())
		        {
		            std::string filePath = request.getLocationConfig().getRoot() + ep->getPath();
		            common::core::raii::UniqueFd fd;
					fd.set(::open(filePath.c_str(), O_RDONLY));
		            if (fd.valid())
		            {
		                char buf[4096];
		                ssize_t n;
		                while ((n = ::read(fd.get(), buf, sizeof(buf))) > 0)
		                    errorBody.append(buf, n);
		                fd.reset();
		                customPageFound = true;
		            }
		            break;
		        }
		    }
		    if (customPageFound) break;
		}

		if (!customPageFound)
		errorBody =	"<html><head><title>"
							+ common::core::utils::toString(error.getStatusCode().getCode())
							+ " "
							+ error.getStatusCode().getMessage()
							+ "</title></head>"
							"<body><h1>"
							+ common::core::utils::toString(error.getStatusCode().getCode())
							+ " "
							+ error.getStatusCode().getMessage()
							+ "</h1><p>"
							+ error.getStatusCode().getDescription()
							+ "</p></body></html>";
		DEBUG(_logger, "errorBody generated, size=" + common::core::utils::toString(errorBody.size()));

		_response.addHeader("Content-Length", common::core::utils::toString(errorBody.size()));
		_response.addHeader("Content-Type", "text/html");
	}

	const t_Headers &headers = _response.getHeaders();
	for (t_Headers::const_reverse_iterator rit = headers.rbegin(); rit != headers.rend(); ++rit)
	{
		const std::list<HTTPheaders::HTTPHeader> &headerList = rit->second;
		for (std::list<HTTPheaders::HTTPHeader>::const_iterator lit = headerList.begin(); lit != headerList.end(); ++lit)
		{
			std::string headerLine = lit->getName() + ": " + lit->getValue() + "\r\n";
			appendToBufferResponse(t_raw(headerLine.begin(), headerLine.end()));
			DEBUG(_logger, "header added to buffer: " + headerLine);
		}
	}
	unsigned char crlf[] = {'\r', '\n'};
	appendToBufferResponse(t_raw(crlf, crlf + 2));
	appendToBufferResponse(t_raw(errorBody.begin(), errorBody.end()));
	DEBUG(_logger, "headers and body serialized, buffer size=" + common::core::utils::toString(_bufferResponse.size()));
}

/**
 * @brief Formats the HTTP status line "HTTP-version SP status-code SP
 *        reason-phrase CRLF" and appends it to the response buffer.
 *
 * @param httpVersion The HTTP version string (e.g. "HTTP/1.1").
 * @param statusCode The status code object providing the numeric code.
 * @param reasonPhrase The textual reason phrase for the status code.
 */
void ResponseHandler::buildStatusLine(const std::string &httpVersion, const status::StatusCode &statusCode, const std::string &reasonPhrase)
{
	//status-line = HTTP-version SP status-code SP [ reason-phrase ]
	std::string statusLine = httpVersion
							+ " "
							+ common::core::utils::toString(statusCode.getCode())
							+ " "
							+ reasonPhrase
							+ "\r\n";
	DEBUG(_logger, httpVersion + " " + common::core::utils::toString(statusCode.getCode()) + " " + reasonPhrase);
	appendToBufferResponse(t_raw(statusLine.begin(), statusLine.end()));
}

/**
 * @brief Adds the standard response headers (Server, Date, Connection) and
 *        manages the session: resolves the Connection header from the request,
 *        and emits a Set-Cookie for an existing or newly created "sid" session.
 *
 * @param request The request providing headers, cookies and session TTL config.
 * @param parsFlags Parser bitmask; when E_PARS_CONNECTION is set, the request's
 *        Connection header is honored to decide keep-alive versus close.
 */
void ResponseHandler::buildHeaders(const client::Request &request, int parsFlags)
{
	DEBUG(_logger, "building response headers");
	_response.addHeader("Server", "webserv/1.0");

	std::time_t t = std::time(NULL);
	std::tm tm = *std::gmtime(&t);
	char dateStr[100];
	std::strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	_response.addHeader("Date", dateStr);

	bool closeConnection = false;
	if (parsFlags & parser::E_PARS_CONNECTION)
	{
		t_Headers::const_iterator it = request.getHeaders().find("connection");
		if (it != request.getHeaders().end())
		{
			const std::list<HTTPheaders::HTTPHeader> &headerList = it->second;
			for (std::list<HTTPheaders::HTTPHeader>::const_iterator lit = headerList.begin();
				lit != headerList.end(); ++lit)
			{
				if (lit->getValue() == "close")
				{
					closeConnection = true;
					break;
				}
			}
		}
	}
	if (closeConnection)
	{
		_response.addHeader("Connection", "close");
		_response.setShouldCloseConnection(true);
	}
	else
	{
		_response.addHeader("Connection", "keep-alive");
	}

	session::SessionStore &store = session::SessionStore::getInstance();
	const t_Cookies &cookies = request.getCookies();
	t_sessionTTL ttl = request.getLocationConfig().getSessionTTL();

	t_Cookies::const_iterator sidIt = cookies.find("sid");
	bool needNewSession = (sidIt == cookies.end());
	if (!needNewSession)
	{
		try {
			session::Session &s = store.getSession(sidIt->second);
			session::Cookie cookie("sid", s.getId());
			cookie.setExpires(s.getExpires());
			_response.addHeader("Set-Cookie", cookie.serializeCookie());
		}
		catch (const std::exception &) {
			needNewSession = true;
		}
	}
	if (needNewSession)
	{
		session::Session newSession = store.createSession(ttl);
		session::Cookie cookie("sid", newSession.getId());
		cookie.setExpires(newSession.getExpires());
		_response.addHeader("Set-Cookie", cookie.serializeCookie());
		DEBUG(_logger, "Set-Cookie: sid=" + newSession.getId());
	}
}

} // !handler
} // !webserv
