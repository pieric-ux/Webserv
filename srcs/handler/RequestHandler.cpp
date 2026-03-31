// TODO: don't forget header

/**
 * @file RequestHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/RequestHandler.hpp>

namespace webserv
{
namespace handler
{

/**
 * @brief [TODO:description]
 */
RequestHandler::RequestHandler()
	:	_request(),
		_bufferRequest(),
		_parser()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.requesthandler");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "RequestHandler instance created");
}

/**
 * @brief [TODO:description]
 */
RequestHandler::~RequestHandler() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
RequestHandler::RequestHandler(const RequestHandler &rhs)
	:	_request(rhs._request),
		_bufferRequest(rhs._bufferRequest),
		_parser(rhs._parser)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
RequestHandler &RequestHandler::operator=(const RequestHandler &rhs)
{
	if (this != &rhs)
	{
		_request = rhs._request;
		_bufferRequest = rhs._bufferRequest;
		_parser = rhs._parser;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	RequestHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.requesthandler");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
client::Request &RequestHandler::getRequest()
{
	return _request;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 */
void RequestHandler::setRequest(const client::Request &request)
{
	_request = request;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
parser::Parser &RequestHandler::getParser()
{
	return _parser;
}

/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 */
void RequestHandler::appendToBufferRequest(const t_raw &buffer)
{
	(void)buffer;
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::clearBufferRequest()
{

}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int RequestHandler::getBodyReceived() const
{
	return 0;
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::parseHeadersFromBufferRequest()
{

}

/**
 * @brief [TODO:description]
 *
 * @param serverConfig [TODO:parameter]
 */
void RequestHandler::validateHeaders(const config::ServerConfig &serverConfig)
{
	(void)serverConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param serverConfig [TODO:parameter]
 */
void RequestHandler::parseBodyFromBuffer(const config::ServerConfig &serverConfig)
{
	(void)serverConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void RequestHandler::buildAbsolutPath(const std::string &requestTarget, const config::LocationConfig &locationConfig)
{
	(void)requestTarget;
	(void)locationConfig;
}

} // !handler
} // !webserv
