// TODO: don't forget header

/**
 * @file RequestHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/RequestHandler.hpp>
#include <webserv/config/DefaultConfig.hpp>

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
		_serverConfig(config),
		_chunkedAvailable(0),
		_chunkedTotalDecoded(0)
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
		_serverConfig(rhs._serverConfig),
		_chunkedAvailable(rhs._chunkedAvailable),
		_chunkedTotalDecoded(rhs._chunkedTotalDecoded)
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
		_chunkedAvailable = rhs._chunkedAvailable;
		_chunkedTotalDecoded = rhs._chunkedTotalDecoded;
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
 */
void RequestHandler::parseHeaders()
{
	DEBUG(_logger, "buffer size=" + common::core::utils::toString(_bufferRequest.size()));

	std::string bufferStr(_bufferRequest.begin(), _bufferRequest.end());

	static const unsigned char CRLF[4] = {'\r', '\n', '\r', '\n'};
	std::string::iterator it = std::search(bufferStr.begin(), bufferStr.end(), CRLF, CRLF + 4);
	if (it == bufferStr.end())
	{
		if (!(_request.getFlags() & client::E_REQ_REQUEST_LINE))
		{
			std::string::size_type firstCrlf = bufferStr.find("\r\n");
			if (firstCrlf == std::string::npos && bufferStr.size() >= config::DefaultConfig::URI_MAX_LENGTH)
			{
				INFO(_logger, "414: URI too long (no CRLF in first " + common::core::utils::toString(config::DefaultConfig::URI_MAX_LENGTH) + "+ bytes)");
				throw client::HTTPError(414);
			}
		}
		if (bufferStr.size() >= config::DefaultConfig::BUFFER_SIZE)
		{
			INFO(_logger, "431: header block exceeds " + common::core::utils::toString(config::DefaultConfig::BUFFER_SIZE) + " bytes");
			throw client::HTTPError(431);
		}
		DEBUG(_logger, "incomplete headers, waiting for more data");
		return ;
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
		_request.setFlags(_request.getFlags() | client::E_REQ_REQUEST_LINE);
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

		_request.setFlags(_request.getFlags() | client::E_REQ_HEADERS_VALIDATED);

		INFO(_logger, "headers validated, remaining buffer=" + common::core::utils::toString(_bufferRequest.size()) + " bytes");
	}
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::parseBody()
{
	if (_parser.getFlags() & parser::E_PARS_TRANSFER_ENCODING)
	{
		if (_request.getFlags() & client::E_REQ_BODY_STARTED)
			return ;
		decodeChunkedBody();
		return ;
	}

	const config::LocationConfig &locationConfig = _request.getLocationConfig();

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

	_request.setFlags(_request.getFlags() | client::E_REQ_BODY_STARTED);
	DEBUG(_logger, "Body started, Content-Length=" + common::core::utils::toString(contentLength));
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
 *
 * @param n [TODO:parameter]
 */	
void RequestHandler::eraseBufferRequestFront(std::size_t n)
{
	if (n >= _bufferRequest.size())
		_bufferRequest.clear();
	else
		_bufferRequest.erase(_bufferRequest.begin(), _bufferRequest.begin() + n);
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::clearBufferRequest()
{
	_bufferRequest.clear();
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
 * @brief Returns how many bytes at the front of the request buffer are
 * already-decoded chunked body data, ready to be consumed.
 *
 * @return Number of consumable decoded bytes.
 */
std::size_t RequestHandler::getChunkedAvailable() const
{
	return _chunkedAvailable;
}

/**
 * @brief Reduces the count of consumable decoded chunked body bytes after a
 * consumer has read and erased n of them from the front of the buffer.
 *
 * @param n Number of bytes that were just consumed.
 */
void RequestHandler::shrinkChunkedAvailable(std::size_t n)
{
	if (n >= _chunkedAvailable)
		_chunkedAvailable = 0;
	else
		_chunkedAvailable -= n;
}

/**
 * @brief Resolves and stores the matching location for the request path, then
 * enforces request semantics: rejects disallowed methods (405) and missing Host
 * (400); for POST/PUT requires either Content-Length or Transfer-Encoding:
 * chunked (411 if neither, 400 if both), validates Content-Type against
 * allowed MIME types or CGI for multipart/form-data (415), and defaults a
 * missing Content-Type to application/octet-stream.
 */
void RequestHandler::validateHeaders()
{
	DEBUG(_logger, "method=" + config::methodToStr(_request.getMethod()) + " target=" + _request.getRequestTarget());

	
	const config::LocationConfig &locationConfig = _serverConfig.findLocationConfig(_request.getPath());
	_request.setLocationConfig(locationConfig);
	
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
		bool hasContentLength = _parser.getFlags() & parser::E_PARS_CONTENT_LENGTH;
		bool hasTransferEncoding = _parser.getFlags() & parser::E_PARS_TRANSFER_ENCODING;

		if (!hasContentLength && !hasTransferEncoding)
		{
			INFO(_logger, "411: missing Content-Length on " + config::methodToStr(_request.getMethod()));
			throw client::HTTPError(411);
		}

		if (hasContentLength && hasTransferEncoding)
		{
			INFO(_logger, "400: Content-Length and Transfer-Encoding both present");
			throw client::HTTPError(400);
		}

		if (!(_parser.getFlags() & parser::E_PARS_CONTENT_TYPE))
		{
			DEBUG(_logger, "no Content-Type, defaulting to application/octet-stream");
			_request.addHeader("Content-Type", "application/octet-stream");
			_parser.setFlags(_parser.getFlags() | parser::E_PARS_CONTENT_TYPE);
		}
	}

	DEBUG(_logger, "all checks passed");
}

/**
 * @brief [TODO:description]
 */
void RequestHandler::buildAbsolutPath()
{
	const config::LocationConfig &locationConfig = _request.getLocationConfig();
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

/**
 * @brief Incrementally decodes a chunked transfer-coding request body.
 *
 * Parses as many complete chunks as are currently available past
 * _chunkedAvailable, stopping without error as soon as a chunk-size line, its
 * data, or the trailer-section is not yet fully buffered (the caller retries
 * on the next tick). Enforces client_max_body_size cumulatively via
 * _chunkedTotalDecoded (413) as each chunk-size is read. Once anything new
 * has been decoded, splices the consumed raw span in _bufferRequest for the
 * decoded bytes and advances _chunkedAvailable. Once the terminating
 * last-chunk and trailer-section are fully consumed, synthesizes a
 * Content-Length header with the final decoded size and marks the body as
 * started so the existing Content-Length-based consumers
 * (ExecutionHandler::writeChunk, CGIHandler) can take over.
 */
void RequestHandler::decodeChunkedBody()
{
	const config::LocationConfig &locationConfig = _request.getLocationConfig();
	t_clientMaxBodySize maxBodySize = locationConfig.getClientMaxBodySize();

	std::string tail(_bufferRequest.begin() + _chunkedAvailable, _bufferRequest.end());
	std::string newlyDecoded;
	std::size_t pos = 0;
	bool complete = false;

	for (;;)
	{
		unsigned long chunkSize = 0;
		std::size_t lineEnd = 0;
		if (!readChunkSizeLine(tail, pos, chunkSize, lineEnd))
			break ;

		if (_chunkedTotalDecoded + newlyDecoded.size() + chunkSize > maxBodySize)
		{
			INFO(_logger, "413: decoded chunked body exceeds client_max_body_size");
			throw client::HTTPError(413);
		}

		if (chunkSize == 0)
		{
			std::size_t newPos = 0;
			if (!readTrailerSection(tail, lineEnd, newPos))
				break ;
			pos = newPos;
			complete = true;
			break ;
		}

		std::string data;
		if (!readChunkData(tail, lineEnd, chunkSize, data))
			break ;

		newlyDecoded += data;
		pos = lineEnd + chunkSize + 2;
	}

	if (!newlyDecoded.empty() || complete)
	{
		_chunkedTotalDecoded += newlyDecoded.size();

		t_raw decoded(newlyDecoded.begin(), newlyDecoded.end());
		_bufferRequest.erase(_bufferRequest.begin() + _chunkedAvailable,
			_bufferRequest.begin() + _chunkedAvailable + pos);
		_bufferRequest.insert(_bufferRequest.begin() + _chunkedAvailable,
			decoded.begin(), decoded.end());
		_chunkedAvailable += newlyDecoded.size();
	}

	if (complete)
	{
		_request.addHeader("Content-Length", common::core::utils::toString(_chunkedTotalDecoded));
		_parser.setFlags(_parser.getFlags() | parser::E_PARS_CONTENT_LENGTH);
		_request.setFlags(_request.getFlags() | client::E_REQ_BODY_STARTED);
		DEBUG(_logger, "Chunked body fully decoded: " + common::core::utils::toString(_chunkedTotalDecoded) + " bytes");
	}
}

/**
 * @brief Reads a chunk-size line (chunk-size [chunk-ext] CRLF) starting at pos
 * in tail. Any chunk-ext after ';' is ignored.
 *
 * @param tail Not-yet-decoded portion of the chunked body.
 * @param pos Position of the chunk-size line within tail.
 * @param chunkSize Set to the parsed chunk size on success.
 * @param lineEnd Set to the position right after the line's CRLF on success.
 * @return true if the line was fully available and valid, false if more data
 *         is needed.
 * @throws client::HTTPError(400) if the line exceeds a sane length without a
 *         CRLF, or if the chunk-size is not valid hexadecimal.
 */
bool RequestHandler::readChunkSizeLine(const std::string &tail, std::size_t pos,
	unsigned long &chunkSize, std::size_t &lineEnd) const
{
	std::string::size_type eol = tail.find("\r\n", pos);
	if (eol == std::string::npos)
	{
		if (tail.size() - pos >= config::DefaultConfig::CLIENT_HEADER_BUFFER_SIZE)
		{
			INFO(_logger, "400: chunk-size line too long or missing CRLF");
			throw client::HTTPError(400);
		}
		return false;
	}

	std::string sizeLine = tail.substr(pos, eol - pos);
	std::string::size_type semicolon = sizeLine.find(';');
	std::string hexPart = (semicolon == std::string::npos) ? sizeLine : sizeLine.substr(0, semicolon);

	errno = 0;
	char *endptr = NULL;
	chunkSize = std::strtoul(hexPart.c_str(), &endptr, 16);
	if (hexPart.empty() || endptr != hexPart.c_str() + hexPart.size() || errno == ERANGE)
	{
		INFO(_logger, "400: invalid chunk-size: " + hexPart);
		throw client::HTTPError(400);
	}

	lineEnd = eol + 2;
	return true;
}

/**
 * @brief Reads chunk-data (1*OCTET CRLF) of chunkSize bytes starting at
 * dataStart in tail.
 *
 * @param tail Not-yet-decoded portion of the chunked body.
 * @param dataStart Position of the chunk-data within tail.
 * @param chunkSize Number of data bytes to read.
 * @param data Set to the chunk-data bytes on success.
 * @return true if the data and its trailing CRLF were fully available, false
 *         if more data is needed.
 * @throws client::HTTPError(400) if the data is not terminated by CRLF.
 */
bool RequestHandler::readChunkData(const std::string &tail, std::size_t dataStart,
	unsigned long chunkSize, std::string &data) const
{
	if (tail.size() < dataStart + chunkSize + 2)
		return false;

	if (tail[dataStart + chunkSize] != '\r' || tail[dataStart + chunkSize + 1] != '\n')
	{
		INFO(_logger, "400: chunk-data not terminated by CRLF");
		throw client::HTTPError(400);
	}

	data = tail.substr(dataStart, chunkSize);
	return true;
}

/**
 * @brief Consumes trailer-section field-lines (validated the same way as
 * regular headers) starting at pos in tail, up to and including the blank
 * line that ends the chunked body.
 *
 * @param tail Not-yet-decoded portion of the chunked body.
 * @param pos Position of the first trailer line (or the terminating blank
 *        line) within tail.
 * @param newPos Set to the position right after the terminating blank line's
 *        CRLF on success.
 * @return true if the whole trailer-section was fully available and valid,
 *         false if more data is needed.
 * @throws client::HTTPError(400) if a trailer line exceeds a sane length
 *         without a CRLF, or fails field-line validation.
 */
bool RequestHandler::readTrailerSection(const std::string &tail, std::size_t pos,
	std::size_t &newPos) const
{
	for (;;)
	{
		std::string::size_type eol = tail.find("\r\n", pos);
		if (eol == std::string::npos)
		{
			if (tail.size() - pos >= config::DefaultConfig::CLIENT_HEADER_BUFFER_SIZE)
			{
				INFO(_logger, "400: trailer line too long or missing CRLF");
				throw client::HTTPError(400);
			}
			return false;
		}

		if (eol == pos)
		{
			newPos = eol + 2;
			return true;
		}

		std::string trailerLine = tail.substr(pos, eol - pos);
		if (!abnf::Abnf::getInstance().match("field-line", "HTTP", trailerLine))
		{
			INFO(_logger, "400: invalid trailer field-line: " + trailerLine.substr(0, 80));
			throw client::HTTPError(400);
		}
		pos = eol + 2;
	}
}

} // !handler
} // !webserv
