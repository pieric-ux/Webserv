// TODO: don't forget header

/**
 * @file ExecutionHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/ExecutionHandler.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>
#include <common/core/utils/Directory.hpp>
#include <common/core/utils/fileUtils.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <cstring>

namespace webserv
{
namespace handler
{

/**
 * @brief [TODO:description]
 */
ExecutionHandler::ExecutionHandler()
	:	_fd(-1),
		_bodyReceived(0),
		_flags(static_cast<e_ExecutionHandlerFlags>(0))
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
		_autoindexBuffer(rhs._autoindexBuffer)
{}

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
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void ExecutionHandler::execute(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	if (locationConfig.getEnableCGI())
	{
		std::string ext = getFileExtension(requestHandler.getRequest().getAbsolutePath());
		const t_CgiExtensions &cgiExts = locationConfig.getCgiExtensions();

		if (cgiExts.find(ext) != cgiExts.end())
		{
			executeCGI(requestHandler, locationConfig);
			return ;
		}
	}
	executeRequest(requestHandler, responseHandler, locationConfig);
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void ExecutionHandler::executeCGI(const RequestHandler &requestHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)locationConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void ExecutionHandler::executeRequest(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	switch (requestHandler.getRequest().getMethod())
	{
		case config::GET:
		case config::HEAD:
			DEBUG(_logger, "executeRequest: executeHEADorGET for method " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executeHEADorGET(requestHandler, responseHandler, locationConfig);
			break;
		case config::POST:
			DEBUG(_logger, "executeRequest: executePOST for method " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executePOST(requestHandler, responseHandler, locationConfig);
			break;
		case config::DELETE:
			DEBUG(_logger, "executeRequest: executeDELETE for method " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executeDELETE(requestHandler, responseHandler, locationConfig);
			break;
		case config::PUT:
			DEBUG(_logger, "executeRequest: executePUT for method " + config::methodToStr(requestHandler.getRequest().getMethod()));
			executePUT(requestHandler, responseHandler, locationConfig);
			break;
		default:
			throw client::HTTPError(405);
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

	DEBUG(_logger, "executeHEADorGET: method=" + config::methodToStr(request.getMethod())
		+ " absPath=\"" + absPath + "\""
		+ " flags=0x" + common::core::utils::toString(getFlags())
		+ " respFlags=0x" + common::core::utils::toString(response.getFlags()));

	// file
	if (isFile(absPath))
	{
		DEBUG(_logger, "executeHEADorGET: target is a file");
		if (!(getFlags() & E_EXEC_FILE_OPENED)) //E_EXEC_FILE_OPENED is NOT set
		{
			openFile(requestHandler, locationConfig);
			int fileSize = getFileSize(absPath);
			response.addHeader("Content-Length", common::core::utils::toString(fileSize)); // k m g ?
			std::string ext = getFileExtension(absPath);
			const t_MimeTypes &types = locationConfig.getTypes();
			t_MimeTypes::const_iterator mimeIt = types.find(ext);
			if (mimeIt != types.end())
				response.addHeader("Content-Type", mimeIt->second);
			else
				response.addHeader("Content-Type", locationConfig.getDefaultType()); // octet-stream
			DEBUG(_logger, "executeHEADorGET: file opened, size=" + common::core::utils::toString(fileSize)
				+ " ext=\"" + ext + "\"");
		}
		if (request.getMethod() == config::GET
			&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
		{
			DEBUG(_logger, "executeHEADorGET: reading chunk (GET body phase)");
			readChunk(responseHandler);
		}
		if (request.getMethod() == config::HEAD
			&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
		{
			DEBUG(_logger, "executeHEADorGET: HEAD complete (no body)");
			setFlags(getFlags() | E_EXEC_COMPLETE);
		}
		return ;
	}

	// dir
	if (isDirectory(absPath))
	{
		DEBUG(_logger, "executeHEADorGET: target is a directory");

		// 301
		if (absPath[absPath.size() - 1] != '/')
		{
			INFO(_logger, "executeHEADorGET: directory without trailing slash, 301 redirect");
			throw client::HTTPError(301);
		}

		// index
		const t_Index &indexes = locationConfig.getIndex();
		for (t_Index::const_iterator it = indexes.begin(); it != indexes.end(); ++it) // index file is present in dir?
		{
			std::string indexPath = absPath + *it;
			if (isFile(indexPath)) // index file found
			{
				DEBUG(_logger, "executeHEADorGET: index file found: \"" + indexPath + "\"");
				request.setAbsolutePath(indexPath);
				if (!(getFlags() & E_EXEC_FILE_OPENED))
				{
					openFile(requestHandler, locationConfig);
					int fileSize = getFileSize(indexPath);
					response.addHeader("Content-Length", common::core::utils::toString(fileSize)); // set content-length for index file  // k m g ?
					std::string ext = getFileExtension(indexPath);
					const t_MimeTypes &types = locationConfig.getTypes();
					t_MimeTypes::const_iterator mimeIt = types.find(ext);
					if (mimeIt != types.end())
						response.addHeader("Content-Type", mimeIt->second);
					else
						response.addHeader("Content-Type", locationConfig.getDefaultType());
					DEBUG(_logger, "executeHEADorGET: index opened, size=" + common::core::utils::toString(fileSize));
				}
				if (request.getMethod() == config::GET
					&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
				{
					DEBUG(_logger, "executeHEADorGET: reading chunk for index file");
					readChunk(responseHandler);
				}
				if (request.getMethod() == config::HEAD
					&& (response.getFlags() & client::E_RESP_HEADERS_SENT))
				{
					DEBUG(_logger, "executeHEADorGET: HEAD complete for index file");
					setFlags(getFlags() | E_EXEC_COMPLETE);
				}
				return ;
			}
		}

		// autoindex and no index  file found
		if (locationConfig.getAutoindex())
		{
			if (!(response.getFlags() & client::E_RESP_HEADERS_SENT)) // E_RESP_HEADERS_SENT is NOT set
			{
				DEBUG(_logger, "executeHEADorGET: generating autoindex HTML");
				std::string html = generateAutoindexHTML(absPath);
				_autoindexBuffer.assign(html.begin(), html.end());
				response.addHeader("Content-Length",
					common::core::utils::toString(static_cast<int>(_autoindexBuffer.size())));
				response.addHeader("Content-Type", "text/html");
			}
			else // E_RESP_HEADERS_SENT is set
			{
				if (request.getMethod() == config::HEAD)
				{
					DEBUG(_logger, "executeHEADorGET: HEAD autoindex complete (no body)");
					_autoindexBuffer.clear();
					setFlags(getFlags() | E_EXEC_COMPLETE);
				}
				else
				{
					DEBUG(_logger, "executeHEADorGET: appending autoindex buffer to response");
					responseHandler.appendToBufferResponse(_autoindexBuffer);
					_autoindexBuffer.clear();
					setFlags(getFlags() | E_EXEC_COMPLETE);
				}
			}
			return ;
		}

		//403
		INFO(_logger, "executeHEADorGET: no index, no autoindex -> 403");
		throw client::HTTPError(403);
	}

	// default 404
	INFO(_logger, "executeHEADorGET: path not found -> 404");
	throw client::HTTPError(404);
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executePOST(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)locationConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeDELETE(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)locationConfig;
}


/**
 * @brief 
 * 
 */

void ExecutionHandler::executePUT(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)locationConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 * @return [TODO:return]
 */
void ExecutionHandler::openFile(const handler::RequestHandler &requestHandler, const config::LocationConfig &locationConfig)
{
	const std::string	&absPath = requestHandler.getRequest().getAbsolutePath();
	const t_Perms		perms = locationConfig.getDavAccess();
	config::e_Method	method = requestHandler.getRequest().getMethod();
	int					flags = 0;
	mode_t				mode = 0;
	int					fd;
	bool				isCreated = false;

	if (method == config::GET || method == config::HEAD)
	{
		flags = O_RDONLY;
		if ((fd = ::open(absPath.c_str(), flags, mode)) < 0)
		{
			int e = errno;
			INFO(_logger, "openFile: open failed on \"" + absPath + "\": " + std::string(std::strerror(e)));
			if (e == ENOENT)
				throw client::HTTPError(404);
			if (e == EACCES)
				throw client::HTTPError(403);
			throw client::HTTPError(500);
		}
		setFlags(getFlags() | E_EXEC_FILE_OPENED);
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
			INFO(_logger, "openFile: open failed on \"" + absPath + "\": " + std::string(std::strerror(e)));
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
		ERROR(_logger, "openFile: unsupported method " + config::methodToStr(method));
		throw client::HTTPError(500);
	}

	_fd.reset(fd);
	DEBUG(_logger, "openFile: opened \"" + absPath + "\" fd=" + common::core::utils::toString(fd));
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
		ERROR(_logger, "readChunk: _fd is not open");
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
		DEBUG(_logger, "readChunk: EOF on fd=" + common::core::utils::toString(_fd.get()));
		return ;
	}
	ERROR(_logger, "readChunk: read() failed on fd=" + common::core::utils::toString(_fd.get()));
	throw client::HTTPError(500);
}

/**
 * @brief Writes at most one BUFFER_SIZE chunk from the request body
 *        (starting at offset _bodyReceived) to the owned _fd. Updates
 *        _bodyReceived on success, throws HTTPError(500) on error or if _fd
 *        is not open.
 *
 * @param request Request whose body provides the bytes to flush.
 */
void ExecutionHandler::writeChunk(const handler::RequestHandler &requestHandler)
{
	if (!_fd.valid())
	{
		ERROR(_logger, "writeChunk: _fd is not open");
		throw client::HTTPError(500);
	}

	const t_raw		&buf = requestHandler.getBufferRequest();
	std::size_t		offset = _bodyReceived;

	const t_Headers	&headers = requestHandler.getRequest().getHeaders();
	t_Headers::const_iterator it = headers.find("Content-Length");
	if (it == headers.end() || it->second.empty())
	{
		ERROR(_logger, "writeChunk: missing Content-Length header");
		throw client::HTTPError(411);
	}

	std::size_t		remaining = std::stoul(it->second.front().getValue()) - offset;
	std::size_t		toWrite = remaining < config::DefaultConfig::BUFFER_SIZE ? remaining : config::DefaultConfig::BUFFER_SIZE;
	ssize_t			wr;

	wr = ::write(_fd.get(), &buf[offset], toWrite);

	if (wr > 0)
	{
		_bodyReceived += wr;
		DEBUG(_logger, "writeChunk: wrote " + common::core::utils::toString(wr) + " bytes to fd=" + common::core::utils::toString(_fd.get()));
		if (_bodyReceived >= static_cast<ssize_t>(std::stoul(it->second.front().getValue())))
			setFlags(getFlags() | E_EXEC_COMPLETE);
		return ;
	}
	ERROR(_logger, "writeChunk: write() failed on fd=" + common::core::utils::toString(_fd.get()));
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
bool ExecutionHandler::isFileExisting(const std::string& path)
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
		INFO(_logger, "deleteFile: failed to remove " + filePath);
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
	catch (const std::exception &e)
	{
		ERROR(_logger, "deleteDirectory: " + std::string(e.what()));
		return;
	}
	if (std::remove(dirPath.c_str()) != 0)
		ERROR(_logger, "deleteDirectory: failed to rmdir " + dirPath);
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
	(void)dirPath;
	return "";
}

} // !handler
} // !webserv
