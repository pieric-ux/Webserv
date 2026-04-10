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
 * @return [TODO:return]
 */
const client::Request &RequestHandler::getRequest() const
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
	_bufferRequest.clear();
}

/**
 * @brief Orchestrate header parsing on the buffered request bytes.
 *
 */
void RequestHandler::parseHeaders()
{
	DEBUG(_logger, "buffer size=" + common::core::utils::toString(_bufferRequest.size()));

	std::string bufferStr(_bufferRequest.begin(), _bufferRequest.end());

	static const unsigned char CRLF[4] = {'\r', '\n', '\r', '\n'};
	std::string::iterator it = std::search(bufferStr.begin(), bufferStr.end(), CRLF, CRLF + 4);
	if (it == bufferStr.end())
	{
		INFO(_logger, "400: CRLFCRLF not found in buffer");
		throw client::HTTPError(400);
	}

	std::string::iterator lineEnd = std::find(bufferStr.begin(), bufferStr.end(), '\n');
	std::string::iterator lineStop = lineEnd;
	if (lineStop != bufferStr.begin() && *(lineStop - 1) == '\r')
		--lineStop;

	std::string line(bufferStr.begin(), lineStop);
	std::string headersBlock(lineEnd + 1, it + 2);

	if (!(_request.getFlags() & client::E_REQ_REQUEST_LINE))
	{
		DEBUG(_logger, "parsing request line: \"" + line + "\"");
		_parser.parseRequestLine(line, _request);
		_request.setFlags(client::E_REQ_REQUEST_LINE);
		DEBUG(_logger, "request line parsed: method=" + config::methodToStr(_request.getMethod()) + " target=" + _request.getRequestTarget());
	}
	if (_request.getFlags() & client::E_REQ_REQUEST_LINE)
	{
		DEBUG(_logger, "parsing headers block (" + common::core::utils::toString(headersBlock.size()) + " bytes)");
		_parser.parseHeaders(headersBlock, _request);
	}

	if (_parser.getFlags() & parser::E_PARS_CLRF && !(_request.getFlags() & client::E_REQ_HEADERS_VALIDATED))
	{
		DEBUG(_logger, "CRLFCRLF detected, starting validation");
		validateHeaders();

		t_raw::iterator end = std::search(_bufferRequest.begin(), _bufferRequest.end(), CRLF, CRLF + 4);
		_bufferRequest.erase(_bufferRequest.begin(), end + 4);

		buildAbsolutPath();

		_request.setFlags(static_cast<client::e_RequestFlags>(_request.getFlags() | client::E_REQ_HEADERS_VALIDATED));

		INFO(_logger, "headers validated, remaining buffer=" + common::core::utils::toString(_bufferRequest.size()) + " bytes");
	}
}

/**
 * @brief [TODO:description]
 *
 * @param serverConfig [TODO:parameter]
 */
void RequestHandler::validateHeaders()
{
	DEBUG(_logger, "method=" + config::methodToStr(_request.getMethod()) + " target=" + _request.getRequestTarget());

	const config::LocationConfig &locationConfig = _serverConfig.findLocationConfig(_request.getPath());

	if (locationConfig.getAllowedMethods().find(_request.getMethod()) == locationConfig.getAllowedMethods().end())
	{
		INFO(_logger, "405: " + config::methodToStr(_request.getMethod()) + " not allowed on " + _request.getPath());
		throw client::HTTPError(405);
	}

	if (!(_parser.getFlags() & parser::E_PARS_HOST))
	{
		INFO(_logger, "400: missing Host header");
		throw client::HTTPError(400);
	}

	if (_request.getMethod() == config::POST || _request.getMethod() == config::PUT)
	{
		if (!(_parser.getFlags() & parser::E_PARS_CONTENT_LENGTH))
		{
			INFO(_logger, "411: missing Content-Length on " + config::methodToStr(_request.getMethod()));
			throw client::HTTPError(411);
		}

		if (_parser.getFlags() & parser::E_PARS_CONTENT_TYPE)
		{
			if (!_request.findHeader("Content-Type", "multipart/form-data").getName().empty())
			{
				DEBUG(_logger, "multipart/form-data detected, checking CGI");
				bool cgiMatch = false;
				const t_CgiExtensions &cgiExt = locationConfig.getCgiExtensions();
				t_CgiExtensions::const_iterator cit = cgiExt.begin();
				for (; cit != cgiExt.end(); ++cit)
				{
					if (common::core::utils::hasExtension(_request.getPath(), cit->first))
					{
						cgiMatch = true;
						break;
					}
				}
				if (!locationConfig.getEnableCGI() || !cgiMatch)
				{
					INFO(_logger, "415: multipart/form-data requires CGI but CGI disabled or no extension match");
					throw client::HTTPError(415);
				}
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
				{
					INFO(_logger, "415: Content-Type not in allowed MIME types");
					throw client::HTTPError(415);
				}
			}
		}
		else
		{
			DEBUG(_logger, "no Content-Type, defaulting to application/octet-stream");
			_request.addHeader("Content-Type", "application/octet-stream");
			_parser.setFlags(_parser.getFlags() | parser::E_PARS_CONTENT_TYPE);
		}

		if (_parser.getFlags() & parser::E_PARS_CONTENT_ENCODING)
		{
			if (!_request.findHeader("Content-Encoding", "chunked").getName().empty())
			{
				INFO(_logger, "415: chunked Content-Encoding not supported");
				throw client::HTTPError(415);
			}
		}
	}

	DEBUG(_logger, "all checks passed");
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::parseBody()
{
	const config::LocationConfig &locationConfig = _serverConfig.findLocationConfig(_request.getPath());

	t_clientMaxBodySize contentLength = 0;
	const t_Headers &headers = _request.getHeaders();
	t_Headers::const_iterator it = headers.find("content-length");
	if (it != headers.end() && !it->second.empty())
	{
		std::istringstream iss(it->second.front().getValue());
		iss >> contentLength;
	}

	t_clientMaxBodySize maxBodySize = locationConfig.getClientMaxBodySize();
	if (contentLength > maxBodySize)
		throw client::HTTPError(413);

	_request.setFlags(static_cast<client::e_RequestFlags>(_request.getFlags() | client::E_REQ_BODY_STARTED));
	DEBUG(_logger, "Body started, Content-Length=" + common::core::utils::toString(contentLength));
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::buildAbsolutPath()
{
	const config::LocationConfig &locationConfig = _serverConfig.findLocationConfig(_request.getPath());
	std::string root;

	if (_request.getMethod() == config::PUT || _request.getMethod() == config::DELETE)
		root = locationConfig.getDavPutPath();
	else
		root = locationConfig.getRoot();

	while (root.size() > 1 && root[root.size() - 1] == '/')
		root = root.substr(0, root.size() - 1);

	std::string absolutePath = normalizePath(root + _request.getPath());

	DEBUG(_logger, "root=" + root + " path=" + _request.getPath() + " -> " + absolutePath);

	if (absolutePath != root &&
		absolutePath.substr(0, root.size() + 1) != root + "/")
	{
		INFO(_logger, "403: path traversal detected: " + absolutePath + " escapes root " + root);
		throw client::HTTPError(403);
	}

	_request.setAbsolutePath(absolutePath);
	DEBUG(_logger, "absolute path set: " + absolutePath);
}

/**
 * @brief [TODO:description]
 *
 * @param path [TODO:parameter]
 * @return [TODO:return]
 */
std::string RequestHandler::normalizePath(const std::string &path)
{
	std::vector<std::string> segments;
	std::istringstream ss(path);
	std::string segment;

	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty() || segment == ".")
			continue;
		else if (segment == "..")
		{
			if (!segments.empty())
				segments.pop_back();
		}
		else
			segments.push_back(segment);
	}

	std::string result;
	std::vector<std::string>::const_iterator it = segments.begin();
	for (; it != segments.end(); ++it)
		result += "/" + *it;

	if (result.empty())
		result = "/";
	else if (path[path.size() - 1] == '/' && result[result.size() - 1] != '/')
		result += "/";

	return result;
}

} // !handler
} // !webserv
