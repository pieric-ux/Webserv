// TODO: don't forget header

/**
 * @file ExecutionHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/ExecutionHandler.hpp>
#include <common/core/utils/Directory.hpp>
#include <common/core/utils/fileUtils.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

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
		_fd(),
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
int ExecutionHandler::getBodyReceived() const
{
	return _bodyReceived;
}

/**
 * @brief [TODO:description]
 *
 * @param bodyReceived [TODO:parameter]
 */
void ExecutionHandler::setBodyReceived(const int bodyReceived)
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
void ExecutionHandler::execute(const RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::ServerConfig &serverConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)serverConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void ExecutionHandler::executeCGI(client::Request &request, const config::ServerConfig &serverConfig)
{
	(void)request;
	(void)serverConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void ExecutionHandler::executeRequest(client::Request &request, client::Response &response, const config::ServerConfig &serverConfig)
{
	(void)request;
	(void)response;
	(void)serverConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeHEADorGET(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig)
{
	(void)request;
	(void)response;
	(void)locationConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executePOST(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig)
{
	(void)request;
	(void)response;
	(void)locationConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param response [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 */
void ExecutionHandler::executeDELETE(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig)
{
	(void)request;
	(void)response;
	(void)locationConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 * @return [TODO:return]
 */
int ExecutionHandler::openFile(const client::Request &request, const config::LocationConfig &locationConfig)
{
	(void)request;
	(void)locationConfig;
	return -1;
}

/**
 * @brief [TODO:description]
 *
 * @param fd [TODO:parameter]
 * @param response [TODO:parameter]
 */
void ExecutionHandler::readChunk(const int fd, client::Response &response)
{
	(void)fd;
	(void)response;
}

/**
 * @brief [TODO:description]
 *
 * @param fd [TODO:parameter]
 * @param request [TODO:parameter]
 */
void ExecutionHandler::writeChunk(const int fd, client::Request &request)
{
	(void)fd;
	(void)request;
}

/**
 * @brief Returns the size of the file at the given path, or -1 if it cannot be accessed.
 *
 * @param requestTarget Path to the file.
 * @return Size of the file, or -1 if it cannot be accessed.
 */
int ExecutionHandler::getFileSize(const std::string &requestTarget)
{
	struct stat	st;

	if (stat(requestTarget.c_str(), &st) != 0)
		return -1;
	return static_cast<int>(st.st_size);
}

/**
 * @brief Returns the file extension (without the dot) of the given path.
 *
 * @param requestTarget Path to inspect.
 * @return Extension string, or empty if none found.
 * @todo TODO: add it to common utils ?
 */
std::string ExecutionHandler::getFileExtension(const std::string &requestTarget)
{
	std::string				name = common::core::utils::filenameNoPath(requestTarget);
	std::string::size_type	dotPos = name.rfind('.');

	if (dotPos == std::string::npos || dotPos == 0)
		return "";
	return name.substr(dotPos + 1);
}

/**
 * @brief Checks whether the given path points to a regular file.
 *
 * @param requestTarget Path to check.
 * @return true if it is a regular file, false otherwise.
 * @todo TODO: add it to common utils ?
 */
bool ExecutionHandler::isFile(const std::string& requestTarget)
{
	struct stat	st;

	if (stat(requestTarget.c_str(), &st) != 0)
		return false;
	return S_ISREG(st.st_mode);
}

/**
 * @brief Checks whether the given path exists on disk.
 *
 * @param requestTarget Path to check.
 * @return true if the path exists, false otherwise.
 * @todo TODO: add it to common utils ?
 */
bool ExecutionHandler::isFileExisting(const std::string& requestTarget)
{
	struct stat	st;

	return (stat(requestTarget.c_str(), &st) == 0);
}

/**
 * @brief Deletes the given file from disk.
 *
 * @param filePath Path to the file to delete.
 */
void ExecutionHandler::deleteFile(const std::string& filePath)
{
	if (unlink(filePath.c_str()) != 0)
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
	if (rmdir(dirPath.c_str()) != 0) // not allowed ?
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
 * @brief Joins a directory path with a file name.
 *
 * @param dir Directory path.
 * @param name File name.
 * @return Combined path.
 * @todo TODO: add it to common utils / move to validations headers
 */
std::string	ExecutionHandler::joinPath(const std::string &dir, const std::string &name)
{
	if (dir.empty())
	return name;
	if (dir[dir.size() - 1] == '/')
	return dir + name;
	return dir + "/" + name;
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
