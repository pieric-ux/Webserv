// TODO: don't forget header

/**
 * @file RequestHandler.cpp
 * @brief [TODO:description]
 */

#include "webserv/client/HTTPError.hpp"
#include "webserv/client/Request.hpp"
#include "webserv/parser/Parser.hpp"
#include <webserv/handler/RequestHandler.hpp>
#include <webserv/client/HTTPError.hpp>
#include <algorithm>

namespace webserv
{
namespace handler
{

/**
 * @brief [TODO:description]
 */
RequestHandler::RequestHandler(config::ServerConfig &config)
	:	_request(),
		_bufferRequest(),
		_parser(),
		_serverConfig(config)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.requesthandler");
	_logger->setLevel(log42::logRecord::DEBUG);
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
	:	_logger(rhs._logger),
		_request(rhs._request),
		_bufferRequest(rhs._bufferRequest),
		_parser(rhs._parser),
		_serverConfig(rhs._serverConfig)
{

}

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
		_logger = rhs._logger;
		_request = rhs._request;
		_bufferRequest = rhs._bufferRequest;
		_parser = rhs._parser;
		_serverConfig = rhs._serverConfig;
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
 * @return [TODO:return]
 */
const t_raw &RequestHandler::getBufferRequest() const
{
	return _bufferRequest;
}

/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 */
void RequestHandler::appendToBufferRequest(const t_raw &buffer)
{
	_bufferRequest.insert(_bufferRequest.end(), buffer.begin(), buffer.end());
	DEBUG(_logger, "Buffer request: +" + common::core::utils::toString(buffer.size()) + " bytes (total=" + common::core::utils::toString(_bufferRequest.size()) + "), buffer content: " + std::string(buffer.begin(), buffer.end()));
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
 * @brief Orchestrate header parsing on the buffered request bytes.
 *
 */
void RequestHandler::parseHeaders()
{
	std::string bufferStr(_bufferRequest.begin(), _bufferRequest.end());

	static const unsigned char CRLF[4] = {'\r', '\n', '\r', '\n'};
	std::string::iterator it = std::search(bufferStr.begin(), bufferStr.end(), CRLF, CRLF + 4);
	if (it == bufferStr.end())
		throw client::HTTPError(400);

	std::string::iterator lineEnd = std::find(bufferStr.begin(), bufferStr.end(), '\n');
	std::string::iterator lineStop = lineEnd;
	if (lineStop != bufferStr.begin() && *(lineStop - 1) == '\r')
		--lineStop;

	std::string line(bufferStr.begin(), lineStop);
	std::string headersBlock(lineEnd + 1, it + 2);

	if (!(_request.getFlags() & client::E_REQ_REQUEST_LINE))
	{
		_parser.parseRequestLine(line, _request);
		_request.setFlags(client::E_REQ_REQUEST_LINE);
	}
	if (_request.getFlags() & client::E_REQ_REQUEST_LINE)
		_parser.parseHeaders(headersBlock, _request);

	if (_parser.getFlags() & parser::E_PARS_CLRF && !(_request.getFlags() & client::E_REQ_HEADERS_VALIDATED))
	{
		validateHeaders();

		t_raw::iterator end = std::search(_bufferRequest.begin(), _bufferRequest.end(), CRLF, CRLF + 4);
		_bufferRequest.erase(_bufferRequest.begin(), end + 4);

		buildAbsolutPath(_request.getRequestTarget(), _serverConfig.findLocationConfig(_request.getRequestTarget()));

		_request.setFlags(static_cast<client::e_RequestFlags>(_request.getFlags() | client::E_REQ_HEADERS_VALIDATED));

		INFO(_logger, "Headers parsed and validated, buffer request updated (remaining bytes: " + common::core::utils::toString(_bufferRequest.size()) + ")");
	}
}

/**
 * @brief [TODO:description]
 *
 * @param serverConfig [TODO:parameter]
 */
void RequestHandler::validateHeaders()
{
	const config::LocationConfig &locationConfig = _serverConfig.findLocationConfig(_request.getRequestTarget());

	t_AllowedMethods::const_iterator it = std::find(locationConfig.getAllowedMethods().begin(), locationConfig.getAllowedMethods().end(), _request.getMethod());
	if (it == locationConfig.getAllowedMethods().end())
		throw client::HTTPError(405);

	if (!(_parser.getFlags() & parser::E_PARS_HOST))
		throw client::HTTPError(400);

	if (_request.getMethod() == config::POST || _request.getMethod() == config::PUT)
	{
		if (!(_parser.getFlags() & parser::E_PARS_CONTENT_LENGTH))
			throw client::HTTPError(411);

		if (_parser.getFlags() & parser::E_PARS_CONTENT_TYPE)
		{
			if (!_request.findHeader("Content-Type", "multipart/form-data").getName().empty())
			{
				bool cgiMatch = false;
				const t_CgiExtensions &cgiExt = locationConfig.getCgiExtensions();
				t_CgiExtensions::const_iterator cit = cgiExt.begin();
				for (; cit != cgiExt.end(); ++cit)
				{
					if (common::core::utils::hasExtension(_request.getRequestTarget(), cit->first))
					{
						cgiMatch = true;
						break;
					}
				}
				if (!locationConfig.getEnableCGI() || !cgiMatch)
					throw client::HTTPError(415);
			}
			else
			{
				bool found = false;
				const t_MimeTypes &types = locationConfig.getTypes();
				t_MimeTypes::const_iterator mit = types.begin();
				for (; mit != types.end(); ++mit)
				{
					if (!_request.findHeader("Content-Type", mit->second).getName().empty())
					{
						found = true;
						break;
					}
				}
				if (!found)
					throw client::HTTPError(415);
			}
		}
		else
		{
			_request.addHeader("Content-Type", "application/octet-stream");
			_parser.setFlags(_parser.getFlags() | parser::E_PARS_CONTENT_TYPE);
		}

		if (_parser.getFlags() & parser::E_PARS_CONTENT_ENCODING)
		{
			if (!_request.findHeader("Content-Encoding", "chunked").getName().empty())
			{
				throw client::HTTPError(415);
			}
		}
	}
}

/**
 * @brief [TODO:description]
 *
 * @param serverConfig [TODO:parameter]
 */
void RequestHandler::parseBody()
{
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
