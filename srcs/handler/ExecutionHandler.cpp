// TODO: don't forget header

/**
 * @file ExecutionHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/ExecutionHandler.hpp>

namespace webserv
{
namespace handler
{

/**
 * @brief [TODO:description]
 */
ExecutionHandler::ExecutionHandler(const t_ioMultiplexer &ioMultiplexer, sockaddr_storage clientAddr)
	:	_fd(-1),
		_bodyReceived(0),
		_flags(static_cast<e_ExecutionHandlerFlags>(0)),
		_autoindexBuffer(),
		_cgiHandler(ioMultiplexer, clientAddr)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.executionhandler");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ExecutionHandler instance created");
}

/**
 * @brief [TODO:description]
 */
ExecutionHandler::~ExecutionHandler() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
ExecutionHandler::ExecutionHandler(const ExecutionHandler &rhs)
	:	_logger(rhs._logger),
		_fd(rhs._fd.get()),
		_bodyReceived(rhs._bodyReceived),
		_flags(rhs._flags),
		_autoindexBuffer(rhs._autoindexBuffer),
		_cgiHandler(rhs._cgiHandler.getIoMultiplexer(), rhs._cgiHandler.getClientAddr())
{
	DEBUG(_logger, "ExecutionHandler copy ctor: rhs.stdinFd=" + common::core::utils::toString(rhs._cgiHandler.getStdinFd())
		+ " rhs.stdoutFd=" + common::core::utils::toString(rhs._cgiHandler.getStdoutFd()));
}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
ExecutionHandler &ExecutionHandler::operator=(const ExecutionHandler &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_fd.reset(-1);
		_bodyReceived = rhs._bodyReceived;
		_flags = rhs._flags;
		_autoindexBuffer = rhs._autoindexBuffer;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	ExecutionHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.executionhandler");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int ExecutionHandler::getFd() const
{
	return _fd.get();
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::size_t ExecutionHandler::getBodyReceived() const
{
	return _bodyReceived;
}

/**
 * @brief [TODO:description]
 *
 * @param bodyReceived [TODO:parameter]
 */
void ExecutionHandler::setBodyReceived(const std::size_t bodyReceived)
{
	_bodyReceived = bodyReceived;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int ExecutionHandler::getFlags() const
{
	return _flags;
}

/**
 * @brief [TODO:description]
 *
 * @param flags [TODO:parameter]
 */
void ExecutionHandler::setFlags(const int flags)
{
	_flags = static_cast<e_ExecutionHandlerFlags>(flags);
}

/**
 * @brief [TODO:description]
 * 
 * @param buffer [TODO:parameter]
 * @return t_raw 
 */
 t_raw ExecutionHandler::getAutoindexBuffer(t_raw &buffer) const
{
	buffer = _autoindexBuffer;
	return _autoindexBuffer;
}
/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 */
void ExecutionHandler::setAutoindexBuffer(const t_raw &buffer)
{
	_autoindexBuffer = buffer;
}

/**
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::execute(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	const config::Return &redirect = locationConfig.getRedirect();
	if (redirect.statusCode.getCode() != 0)
	{
		INFO(_logger, "return directive " + common::core::utils::toString(redirect.statusCode.getCode())
			+ (redirect.url.empty() ? "" : " -> " + redirect.url));
		throw client::HTTPError(redirect.statusCode.getCode(), redirect.url);
	}

	if (locationConfig.isEnableCGI())
	{
		std::string ext = "." + getFileExtension(requestHandler.getRequest().getAbsolutePath());
		const t_CgiExtensions &cgiExts = locationConfig.getCgiExtensions();

		if (cgiExts.find(ext) != cgiExts.end())
			return ;
	}
	executeRequest(requestHandler, responseHandler, locationConfig);
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void ExecutionHandler::executeCGI(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	if (!_cgiHandler.isSpawned())
	{
		DEBUG(_logger, "executeCGI: not spawned yet, spawning");
		_cgiHandler.spawn(requestHandler.getRequest(), locationConfig);
		setFlags(getFlags() | E_EXEC_CGI_SPAWNED);
		return ;
	}

	_cgiHandler.driveIO(requestHandler, getFlags());

	if (_cgiHandler.isReaped() && !_cgiHandler.isParsed())
	{
		DEBUG(_logger, "executeCGI: child reaped, parsing response");
		_cgiHandler.parse(responseHandler.getResponse());
		setFlags(getFlags() | E_EXEC_COMPLETE);
		DEBUG(_logger, "executeCGI: E_EXEC_COMPLETE set");
		return ;
	}
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
CGIHandler &ExecutionHandler::getCgi()
{
	return _cgiHandler;
}

/**
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeRequest(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	switch (requestHandler.getRequest().getMethod())
	{
		case config::GET:
		case config::HEAD:
			DEBUG(_logger, "dispatching " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executeHEADorGET(requestHandler, responseHandler, locationConfig);
			break;
		case config::POST:
			DEBUG(_logger, "dispatching " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executePOST(requestHandler, locationConfig);
			break;
		case config::PUT:
			DEBUG(_logger, "dispatching " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executePUT(requestHandler, locationConfig);
			break;
		case config::DELETE:
			DEBUG(_logger, "dispatching " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executeDELETE(requestHandler, locationConfig);
			break;
		default:
		{
			INFO(_logger, "405: method not allowed: " + config::methodToStr(requestHandler.getRequest().getMethod()));
			throw client::HTTPError(405);
		}
	}
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeHEADorGET(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	client::Request		&request = requestHandler.getRequest();
	client::Response	&response = responseHandler.getResponse();
	std::string			absPath = request.getAbsolutePath();

	DEBUG(_logger, "method=" + config::methodToStr(request.getMethod())
		+ " absPath=\"" + absPath + "\""
		+ " flags=0x" + common::core::utils::toString(getFlags())
		+ " respFlags=0x" + common::core::utils::toString(response.getFlags()));

	// dir
	if (isDirectory(absPath))
	{
		DEBUG(_logger, "target is a directory");

		// 301
		if (absPath[absPath.size() - 1] != '/')
		{
			INFO(_logger, "301: directory without trailing slash, redirect");
			throw client::HTTPError(301, request.getRequestTarget() + "/");
		}

		// index
		const t_Index &indexes = locationConfig.getIndex();
		for (t_Index::const_iterator it = indexes.begin(); it != indexes.end(); ++it) // index file is present in dir?
		{
			std::string indexPath = absPath + *it;
			if (isExisting(indexPath) && !isDirectory(indexPath)) // index file found
			{
				DEBUG(_logger, "index file found: \"" + indexPath + "\"");
				request.setAbsolutePath(indexPath);
				if (!(getFlags() & E_EXEC_FILE_OPENED))
				{
					openFile(requestHandler, locationConfig);
					int fileSize = getFileSize(indexPath);
					response.addHeader("Content-Length", common::core::utils::toString(fileSize)); // set content-length for index file  
					const t_MimeTypes &types = locationConfig.getTypes();
					t_MimeTypes::const_iterator mimeIt = types.find(getFileExtension(indexPath));
					if (mimeIt != types.end())
						response.addHeader("Content-Type", mimeIt->second);
					else
						response.addHeader("Content-Type", locationConfig.getDefaultType());
					DEBUG(_logger, "index opened, size=" + common::core::utils::toString(fileSize));
				}
				if (request.getMethod() == config::GET
					&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
				{
					DEBUG(_logger, "reading chunk for index file");
					readChunk(responseHandler);
				}
				if (request.getMethod() == config::HEAD
					&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
				{
					setFlags(getFlags() | E_EXEC_COMPLETE);
					DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after HEAD index file");
				}
				return ;
			}
		}

		// autoindex and no index file found
		if (locationConfig.getAutoindex())
		{
			if (!(response.getFlags() & client::E_RESP_HEADERS_SENT)) // E_RESP_HEADERS_SENT is NOT set
			{
				DEBUG(_logger, "generating autoindex HTML");
				std::string html = generateAutoindexHTML(absPath);
				setAutoindexBuffer(t_raw(html.begin(), html.end()));
				response.addHeader("Content-Length",common::core::utils::toString(static_cast<int>(_autoindexBuffer.size())));
				response.addHeader("Content-Type", "text/html; charset=utf-8");
			}
			else // E_RESP_HEADERS_SENT is set
			{
				if (request.getMethod() == config::HEAD)
				{
					DEBUG(_logger, "HEAD autoindex complete (no body)");
				}
				else
				{
					DEBUG(_logger, "appending autoindex buffer to response");
					responseHandler.appendToBufferResponse(_autoindexBuffer);
				}
				_autoindexBuffer.clear();
				setFlags(getFlags() | E_EXEC_COMPLETE);
				DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after autoindex");
			}
			return ;
		}

		//403
		INFO(_logger, "403: no index, no autoindex");
		throw client::HTTPError(403);
	}

	// file
	if (isExisting(absPath))
	{
		DEBUG(_logger, "target is a file");
		if (!(getFlags() & E_EXEC_FILE_OPENED)) //E_EXEC_FILE_OPENED is NOT set
		{
			openFile(requestHandler, locationConfig);
			int fileSize = getFileSize(absPath);
			response.addHeader("Content-Length", common::core::utils::toString(fileSize));
			const t_MimeTypes &types = locationConfig.getTypes();
			t_MimeTypes::const_iterator mimeIt = types.find( getFileExtension(absPath));
			if (mimeIt != types.end())
				response.addHeader("Content-Type", mimeIt->second);
			else
				response.addHeader("Content-Type", locationConfig.getDefaultType()); // octet-stream
			DEBUG(_logger, "file opened, size=" + common::core::utils::toString(fileSize)
				+ " ext=\"" + getFileExtension(absPath) + "\"");
		}
		if (request.getMethod() == config::GET
			&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
		{
			DEBUG(_logger, "reading chunk (GET body phase)");
			readChunk(responseHandler);
		}
		if (request.getMethod() == config::HEAD
			&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
		{
			setFlags(getFlags() | E_EXEC_COMPLETE);
			DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after HEAD file");
		}
		return ;
	}

	// default 404
	INFO(_logger, "404: path not found");
	throw client::HTTPError(404);
}

/**
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executePOST(const RequestHandler &requestHandler, const config::LocationConfig &locationConfig)
{
	std::string absPath = requestHandler.getRequest().getAbsolutePath();
	std::string root = locationConfig.getRoot();
	while(root.size() > 1 && root[root.size() - 1] == '/')
		root = root.substr(0, root.size() - 1);

	if (isDirectory(absPath))
	{
		if (absPath == root + '/')
		{
			INFO(_logger, "403: POST to root directory is not allowed");
			throw client::HTTPError(403);
		}
		if (absPath[absPath.size() - 1] != '/')
		{
			INFO(_logger, "301: directory without trailing slash, redirect");
			throw client::HTTPError(301, requestHandler.getRequest().getRequestTarget() + "/");
		}
		
		const t_Index &indexes = locationConfig.getIndex();

		t_Index::const_iterator it = indexes.begin();
		for (; it != indexes.end(); ++it)
		{
			if (isExisting(absPath + *it))
			{
				INFO(_logger, "403: index file found for POST target, but POST to index file is not allowed");
				throw client::HTTPError(403);
			}
		}
		INFO(_logger, "405: target is a directory, but no index file found and POST to directory is not allowed");
		throw client::HTTPError(405);
	}
	else if (isExisting(absPath))
	{
		INFO(_logger, "405: target is an existing file, but POST to existing file is not allowed");
		throw client::HTTPError(405);
	}
	else
	{
		INFO(_logger, "404: target is a non-existing file, but POST to non-existing file is not allowed");
		throw client::HTTPError(404);
	}
}

/**
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executePUT(RequestHandler &requestHandler, const config::LocationConfig &locationConfig)
{
	t_DavMethods davMethods = locationConfig.getDavMethods();

	if (davMethods.find(config::PUT) == davMethods.end())
		throw client::HTTPError(405);

	const std::string &absPath = requestHandler.getRequest().getAbsolutePath();

	if (absPath[absPath.size() - 1] == '/')
	{
		INFO(_logger, "409: PUT to directory is not allowed");
		throw client::HTTPError(409);
	}
	if (isDirectory(absPath))
	{
		INFO(_logger, "409: PUT to directory is not allowed");
		throw client::HTTPError(409);
	}

	if (!(getFlags() & E_EXEC_FILE_OPENED))
		openFile(requestHandler, locationConfig);
	writeChunk(requestHandler);
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeDELETE(const RequestHandler &requestHandler, const config::LocationConfig &locationConfig)
{
	std::string absPath = requestHandler.getRequest().getAbsolutePath();
	t_DavMethods davMethods = locationConfig.getDavMethods();

	if (davMethods.find(config::DELETE) == davMethods.end())
	{
		INFO(_logger, "405: DELETE method not allowed by dav_methods");
		throw client::HTTPError(405);
	}

	if (absPath[absPath.size() - 1] == '/')
	{
		if (!isExisting(absPath))
		{
			INFO(_logger, "404: target directory not found");
			throw client::HTTPError(404);
		}
		if (!isDirectory(absPath))
		{
			INFO(_logger, "409: target is not a directory");
			throw client::HTTPError(409);
		}
		deleteDirectory(absPath);
	}
	else if (isDirectory(absPath))
	{
		INFO(_logger, "409: target is a directory, but DELETE to directory is not allowed");
		throw client::HTTPError(409);
	}
	else
		deleteFile(absPath);

	setFlags(getFlags() | E_EXEC_NOCONTENT);
	setFlags(getFlags() | E_EXEC_COMPLETE);
	DEBUG(_logger, "E_EXEC_COMPLETE flags set in execution handler after DELETE");
}

/**
 * @brief Opens the file targeted by @p request and stores the resulting fd
 *        in the owned _fd (UniqueFd). Sets E_EXEC_FILE_OPENED on success.
 *
 *        The flow is "check before open" so we never touch open() unless we
 *        already know the operation is allowed both by the webserv config
 *        and by the underlying filesystem:
 *
 *        1. Check the location's `dav_access` bits (config-level rule).
 *           Only the 'all' triplet (least significant 3 bits) is consulted
 *           because HTTP has no authenticated user concept here.
 *           - GET / HEAD require the read bit  (0004) -> else 403
 *           - PUT        requires the write bit (0002) -> else 403
 *
 *        2. Probe the filesystem with access():
 *           - GET / HEAD : access(R_OK) -> ENOENT=404, EACCES=403, else=500
 *           - PUT        : access(F_OK) to distinguish create vs overwrite,
 *                          then access(W_OK) on overwrite.
 *
 *        3. Only then call open() with the appropriate flags. mode for
 *           PUT is derived from `dav_access` directly (mode_t-compatible).
 *
 * @param request        Request providing method and absolute path.
 * @param locationConfig Location whose `dav_access` drives both the
 *                       allow check and the create mode.
 * @return The opened file descriptor (also stored in _fd).
 * @throws client::HTTPError on failure.
 */
void ExecutionHandler::openFile(const handler::RequestHandler &requestHandler, const config::LocationConfig &locationConfig)
{
	const std::string	&absPath = requestHandler.getRequest().getAbsolutePath();
	const t_Perms		perms = locationConfig.getDavAccess();
	config::e_Method	method = requestHandler.getRequest().getMethod();
	int					flags = 0;
	mode_t				mode = 0;
	int					fd;
	bool				isCreated = true;

	if (method == config::GET || method == config::HEAD)
	{
		flags = O_RDONLY;
		if ((fd = ::open(absPath.c_str(), flags, mode)) < 0)
		{
			int e = errno;
			ERROR(_logger, "500: open failed on \"" + absPath + "\": " + std::string(std::strerror(e)));
			if (e == ENOENT)
			{
				INFO(_logger, "404: file not found");
				throw client::HTTPError(404);
			}
			if (e == EACCES)
			{
				INFO(_logger, "403: access denied");
				throw client::HTTPError(403);
			}
			ERROR(_logger, "500: open failed");
			throw client::HTTPError(500);
		}
	}
	else if (method == config::PUT)
	{
		flags = O_WRONLY | O_CREAT | O_TRUNC;
		mode = static_cast<mode_t>(perms);
		if (::access(absPath.c_str(), F_OK) == 0)
		{
			isCreated = false;
		}
		if ((fd = ::open(absPath.c_str(), flags, mode)) < 0)
		{
			int e = errno;
			ERROR(_logger, "open failed on \"" + absPath + "\": " + std::string(std::strerror(e)));
			if (e != ENOENT)
				throw client::HTTPError(500);
		}
		if (isCreated)
			setFlags(getFlags() | E_EXEC_CREATED);
		else
			setFlags(getFlags() | E_EXEC_NOCONTENT);
	}
	else
	{
		ERROR(_logger, "500: unsupported method " + config::methodToStr(method));
		throw client::HTTPError(500);
	}

	setFlags(getFlags() | E_EXEC_FILE_OPENED);

	_fd.reset(fd);
	DEBUG(_logger, "opened \"" + absPath + "\" fd=" + common::core::utils::toString(fd));
}

/**
 * @brief Reads at most one EXECUTION_CHUNK_SIZE chunk from the owned _fd and
 *        appends it to the response body. Sets E_EXEC_COMPLETE on EOF, throws
 *        HTTPError(500) on read error or if _fd is not open.
 *
 * @param response Response whose body buffer is appended to.
 */
void ExecutionHandler::readChunk(handler::ResponseHandler &responseHandler)
{
	if (!_fd.valid())
	{
		ERROR(_logger, "500: _fd is not open");
		throw client::HTTPError(500);
	}

	unsigned char	buf[config::DefaultConfig::BUFFER_SIZE];
	ssize_t			rd;

	rd = ::read(_fd.get(), buf, sizeof(buf));
	if (rd > 0)
	{
		t_raw	chunk(buf, buf + rd);
		responseHandler.appendToBufferResponse(chunk);
		return ;
	}
	if (rd == 0)
	{
		setFlags(getFlags() | E_EXEC_COMPLETE);
		DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after EOF on file");
		DEBUG(_logger, "EOF on fd=" + common::core::utils::toString(_fd.get()));
		_fd.reset();
		return ;
	}
	ERROR(_logger, "500: read() failed on fd=" + common::core::utils::toString(_fd.get()));
	throw client::HTTPError(500);
}

/**
 * @brief Writes one BUFFER_SIZE-capped slice of the request body to the owned
 *        _fd, dispatching to the Transfer-Encoding: chunked streaming writer
 *        or the Content-Length-based writer depending on the request.
 *        Throws HTTPError(500) if _fd is not open.
 *
 * @param request Request whose body provides the bytes to flush.
 */
void ExecutionHandler::writeChunk(handler::RequestHandler &requestHandler)
{
	if (!_fd.valid())
	{
		ERROR(_logger, "500: _fd is not open");
		throw client::HTTPError(500);
	}

	if (requestHandler.getParser().getFlags() & parser::E_PARS_TRANSFER_ENCODING)
		writeChunkChunked(requestHandler);
	else
		writeChunkContentLength(requestHandler);
}

/**
 * @brief Writes at most one BUFFER_SIZE chunk of the buffered request body to
 *        the owned _fd, capped by the remaining Content-Length. Consumes the
 *        written bytes from the request buffer and updates _bodyReceived; on
 *        completion sets E_EXEC_COMPLETE and releases _fd. Throws HTTPError(411)
 *        if Content-Length is missing and HTTPError(500) on write error.
 *
 * @param requestHandler Request handler whose request body buffer provides the
 *                       bytes to flush.
 */
void ExecutionHandler::writeChunkContentLength(handler::RequestHandler &requestHandler)
{
	const t_raw		&buf = requestHandler.getBufferRequest();

	const t_Headers	&headers = requestHandler.getRequest().getHeaders();
	t_Headers::const_iterator it = headers.find("content-length");
	if (it == headers.end() || it->second.empty())
	{
		INFO(_logger, "411: missing Content-Length header");
		throw client::HTTPError(411);
	}

	ssize_t			contentLengthVal = std::strtoul(it->second.front().getValue().c_str(), NULL, 10);
	std::size_t		remaining = contentLengthVal - _bodyReceived;
	std::size_t		available = buf.size();
	if (available == 0)
	{
		if (_bodyReceived >= contentLengthVal)
		{
			setFlags(getFlags() | E_EXEC_COMPLETE);
			DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after all body received");
			_fd.reset();
			_bodyReceived = 0;
		}
		return ;
	}
	std::size_t		toWrite = std::min(remaining, std::min(config::DefaultConfig::BUFFER_SIZE, available));

	DEBUG(_logger, "buff request :" + std::string(buf.begin(), buf.end()));

	ssize_t			wr;
	wr = ::write(_fd.get(), &buf[0], toWrite);

	if (wr > 0)
	{
		requestHandler.eraseBufferRequestFront(wr);
		_bodyReceived += wr;
		DEBUG(_logger, "wrote " + common::core::utils::toString(wr) + " bytes to fd=" + common::core::utils::toString(_fd.get()));
		if (_bodyReceived >= contentLengthVal)
		{
			setFlags(getFlags() | E_EXEC_COMPLETE);
			DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after all body received");
			_fd.reset();
			_bodyReceived = 0;
		}
		return ;
	}
	_bodyReceived = 0;
	ERROR(_logger, "500: write() failed on fd=" + common::core::utils::toString(_fd.get()));
	throw client::HTTPError(500);
}

/**
 * @brief Writes at most one BUFFER_SIZE slice of the chunked request body
 *        that RequestHandler has already decoded and made available, without
 *        ever needing to know the total body size upfront. Completes once no
 *        more decoded bytes are available and RequestHandler has signalled
 *        the chunked body as fully received. Throws HTTPError(500) on write
 *        error.
 *
 * @param requestHandler Request handler tracking the decoded chunked body
 *                       bytes available for consumption.
 */
void ExecutionHandler::writeChunkChunked(handler::RequestHandler &requestHandler)
{
	std::size_t available = std::min(requestHandler.getChunkedAvailable(), config::DefaultConfig::BUFFER_SIZE);
	if (available == 0)
	{
		if (requestHandler.getRequest().getFlags() & client::E_REQ_BODY_STARTED)
		{
			setFlags(getFlags() | E_EXEC_COMPLETE);
			DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after chunked body fully consumed");
			_fd.reset();
		}
		return ;
	}

	const t_raw	&buf = requestHandler.getBufferRequest();
	ssize_t		wr = ::write(_fd.get(), &buf[0], available);

	if (wr > 0)
	{
		requestHandler.eraseBufferRequestFront(wr);
		requestHandler.shrinkChunkedAvailable(wr);
		DEBUG(_logger, "wrote " + common::core::utils::toString(wr) + " bytes (chunked) to fd=" + common::core::utils::toString(_fd.get()));
		return ;
	}
	ERROR(_logger, "500: write() failed on fd=" + common::core::utils::toString(_fd.get()));
	throw client::HTTPError(500);
}

/**
 * @brief Returns the size of the file at the given path, or -1 if it cannot be accessed.
 *
 * @param path Path to the file.
 * @return Size of the file, or -1 if it cannot be accessed.
 */
int ExecutionHandler::getFileSize(const std::string &path)
{
	struct stat	st;

	if (stat(path.c_str(), &st) != 0)
		return -1;
	return static_cast<int>(st.st_size);
}

/**
 * @brief Returns the file extension (without the dot) of the given path.
 *
 * @param path Path to inspect.
 * @return Extension string, or empty if none found.
 * @todo TODO: add it to common utils ?
 */
std::string ExecutionHandler::getFileExtension(const std::string &path)
{
	std::string				name = common::core::utils::filenameNoPath(path);
	std::string::size_type	dotPos = name.rfind('.');

	if (dotPos == std::string::npos || dotPos == 0)
		return "";
	return name.substr(dotPos + 1);
}

/**
 * @brief Checks whether the given path points to a regular file.
 *
 * @param path Path to check.
 * @return true if it is a regular file, false otherwise.
 * @todo TODO: add it to common utils ?
 */
bool ExecutionHandler::isFile(const std::string& path)
{
	struct stat	st;

	if (stat(path.c_str(), &st) != 0)
		return false;
	return S_ISREG(st.st_mode);
}

/**
 * @brief Checks whether the given path exists on disk.
 *
 * @param path Path to check.
 * @return true if the path exists, false otherwise.
 * @todo TODO: add it to common utils ?
 */
bool ExecutionHandler::isExisting(const std::string& path)
{
	struct stat	st;

	return (stat(path.c_str(), &st) == 0);
}

/**
 * @brief Deletes the given file from disk.
 *
 * @param filePath Path to the file to delete.
 */
void ExecutionHandler::deleteFile(const std::string& filePath)
{
	if (std::remove(filePath.c_str()) != 0)
	{
		int e = errno;
		ERROR(_logger, "500: failed to unlink \"" + filePath + "\": " + std::string(std::strerror(e)));
		if (e == ENOENT)
		{
			INFO(_logger, "404: file not found");
			throw client::HTTPError(404);
		}
		if (e == EACCES)
		{
			INFO(_logger, "403: access denied");
			throw client::HTTPError(403);
		}
		ERROR(_logger, "500: remove failed");
		throw client::HTTPError(500);
	}
}

/**
 * @brief Recursively deletes the given directory and its contents.
 *
 * @param dirPath Path to the directory to delete.
 * @todo TODO: add it to common utils ?
 */
void ExecutionHandler::deleteDirectory(const std::string& dirPath)
{
	try
	{
		common::core::utils::Directory				dir(dirPath);
		common::core::utils::DirectoryIterator		it = dir.begin();
		common::core::utils::DirectoryIterator		end = dir.end();

		for (; it != end; ++it)
		{
			std::string	name((*it)->d_name);
			if (name == "." || name == "..")
				continue;
			std::string	path = dirPath;
			if (!path.empty() && path[path.size() - 1] != '/')
				path += "/";
			path += name;

			if (isFile(path))
				deleteFile(path);
			else
				deleteDirectory(path);
		}
	}
	catch (const client::HTTPError &e)
	{
		ERROR(_logger, std::string(e.what()));
		throw;
	}
	catch (const std::exception &e)
	{
		ERROR(_logger, "500: " + std::string(e.what()));
		throw client::HTTPError(500);
	}

	if (std::remove(dirPath.c_str()) != 0)
	{
		int e = errno;
		ERROR(_logger, "500: failed to rmdir \"" + dirPath + "\": " + std::string(std::strerror(e)));
		if (e == ENOENT)
		{
			INFO(_logger, "404: directory not found");
			throw client::HTTPError(404);
		}
		if (e == EACCES)
		{
			INFO(_logger, "403: access denied");
			throw client::HTTPError(403);
		}
		ERROR(_logger, "500: remove failed");
		throw client::HTTPError(500);
	}
}

/**
 * @brief Checks whether the given path points to a directory.
 *
 * @param path Path to check.
 * @return true if it is a directory, false otherwise.
 */
bool	ExecutionHandler::isDirectory(const std::string &path)
{
	struct stat st;
	if (::stat(path.c_str(), &st) != 0)
		return false;
	return S_ISDIR(st.st_mode);
}

/**
 * @brief [TODO:description]
 *
 * @param dirPath [TODO:parameter]
 * @return [TODO:return]
 */
std::string ExecutionHandler::generateAutoindexHTML(const std::string &dirPath)
{
	std::string html = "<html><head><title>Index of "
						+ dirPath
						+ "</title></head><body><h1>Index of "
						+ dirPath
						+ "</h1><ul>";

	DEBUG(_logger, "generating autoindex HTML for \"" + dirPath + "\"\n curent HTML:\n" + html);

	try
	{
		common::core::utils::Directory				dir(dirPath);
		common::core::utils::DirectoryIterator		it = dir.begin();
		common::core::utils::DirectoryIterator		end = dir.end();

		for (; it != end; ++it)
		{
			std::string	name((*it)->d_name);
			if (name == ".")
				continue;
			std::string	path = dirPath;
			if (!path.empty() && path[path.size() - 1] != '/')
				path += "/";
			path += name;
			std::string tmp = "<li><a href=\"" + name + (isDirectory(path) ? "/" : "") + "\">" + name + "</a></li>";
			DEBUG(_logger, tmp);
			html += tmp;
		}
	}
	catch (const std::exception &e)
	{
		ERROR(_logger, "500: " + std::string(e.what()));
		throw client::HTTPError(500);
		return "";
	}

	std::string tail = "</ul></body></html>";
	DEBUG(_logger, tail);	
	html += tail;
	DEBUG(_logger, "generated autoindex HTML for \"" + dirPath + "\"");
 
	return html;
}

} // !handler
} // !webserv
