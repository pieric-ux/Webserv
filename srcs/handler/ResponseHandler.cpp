// TODO: don't forget header

/**
 * @file ResponseHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/ResponseHandler.hpp>

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
	(void)buffer;
}

/**
 * @brief [TODO:description]
 */
void ResponseHandler::clearBufferResponse()
{

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
	(void)httpVersion;
	(void)statusCode;
	(void)reasonPhrase;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 */
void ResponseHandler::buildHeaders(const client::Request &request)
{
	(void)request;
}

} // !handler
} // !webserv
