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
		_flags(rhs._flags)
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
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::execute(const RequestHandler &requestHandler, const ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
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
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeRequest(const RequestHandler &requestHandler, const ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
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
			executePOST(requestHandler, locationConfig);
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
void ExecutionHandler::executeHEADorGET(const RequestHandler &requestHandler, const ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)locationConfig;
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
			throw client::HTTPError(403);
		if (absPath[absPath.size() - 1] != '/')
			throw client::HTTPError(301);
		
		const t_Index &indexes = locationConfig.getIndex();

		t_Index::const_iterator it = indexes.begin();
		for (; it != indexes.end(); ++it)
		{
			if (isExisting(absPath + *it))
				throw client::HTTPError(403);
		}
		throw client::HTTPError(405);
	}
	else if (isExisting(absPath))
		throw client::HTTPError(405);
	else
		throw client::HTTPError(404);
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeDELETE(const RequestHandler &requestHandler, const ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)locationConfig;
}


/**
 * @brief 
 * 
 */

void ExecutionHandler::executePUT(const RequestHandler &requestHandler, const ResponseHandler &responseHandler, const config::LocationConfig &locationConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)locationConfig;
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
