// TODO: don't forget header

/**
 * @file ResponseHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/ResponseHandler.hpp>
#include <webserv/handler/ExecutionHandler.hpp>

namespace webserv
{
namespace handler
{

/**
 * @brief [TODO:description]
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
 * @brief [TODO:description]
 */
ResponseHandler::~ResponseHandler() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
ResponseHandler::ResponseHandler(const ResponseHandler &rhs)
	:	_logger(rhs._logger),
		_response(rhs._response),
		_bufferResponse(rhs._bufferResponse)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
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
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	ResponseHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.responsehandler");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
client::Response &ResponseHandler::getResponse()
{
	return _response;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const client::Response &ResponseHandler::getResponse() const
{
	return _response;
}

/**
 * @brief [TODO:description]
 *
 * @param response [TODO:parameter]
 */
void ResponseHandler::setResponse(const client::Response &response)
{
	_response = response;
}

/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 */
void ResponseHandler::appendToBufferResponse(const t_raw &buffer)
{
	_bufferResponse.insert(_bufferResponse.end(), buffer.begin(), buffer.end());
}

/**
 * @brief [TODO:description]
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
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 */
void ResponseHandler::buildHeadersResponse(const client::Request &request, int execFlags, int parsFlags)
{
	DEBUG(_logger, "execFlags=0x" + common::core::utils::toString(execFlags)
		+ " parsFlags=0x" + common::core::utils::toString(parsFlags));
	
	// set status codes
	if (execFlags & E_EXEC_CREATED)
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(201));
	else if (execFlags & E_EXEC_NOCONTENT)
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(204));
	else
		_response.setStatusCode(status::StatusCodeRegistry::getInstance().getStatusCode(200));
	DEBUG(_logger, "status code set to " + common::core::utils::toString(_response.getStatusCode().getCode()));
	
	buildStatusLine(request.getHttpVersion(), _response.getStatusCode(), _response.getStatusCode().getMessage());
	buildHeaders(request, parsFlags);

	int code = _response.getStatusCode().getCode();
	if (code == 201)
		_response.addHeader("Content-Length", "0");

	//append headers to bufferReponse
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
	DEBUG(_logger, "headers serialized, buffer size=" + common::core::utils::toString(_bufferResponse.size()));
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param error [TODO:parameter]
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
 * @brief [TODO:description]
 *
 * @param httpVersion [TODO:parameter]
 * @param statusCode [TODO:parameter]
 * @param reasonPhrase [TODO:parameter]
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
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
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
}

} // !handler
} // !webserv
