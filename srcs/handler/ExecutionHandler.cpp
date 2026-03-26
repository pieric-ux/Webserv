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
ExecutionHandler::ExecutionHandler()
	:	_fd(-1),
		_bodyReceived(0),
		_flags(static_cast<e_ExecutionHandlerFlags>(0))
{}

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
	:	_fd(rhs._fd),
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
		_fd = rhs._fd;
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
int ExecutionHandler::getFd() const
{
	return _fd;
}

/**
 * @brief [TODO:description]
 *
 * @param fd [TODO:parameter]
 */
void ExecutionHandler::setFd(const int fd)
{
	_fd = fd;
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
void ExecutionHandler::execute(client::Request &request, client::Response &response, const config::ServerConfig &serverConfig)
{
	(void)request;
	(void)response;
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
 * @brief [TODO:description]
 *
 * @param fd [TODO:parameter]
 * @return [TODO:return]
 */
int ExecutionHandler::getFileSize(const int fd)
{
	(void)fd;
	return 0;
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 * @return [TODO:return]
 */
std::string ExecutionHandler::getFileExtension(const std::string &requestTarget)
{
	(void)requestTarget;
	return "";
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 * @return [TODO:return]
 */
bool ExecutionHandler::isFile(const std::string& requestTarget)
{
	(void)requestTarget;
	return false;
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 * @return [TODO:return]
 */
bool ExecutionHandler::isFileExisting(const std::string& requestTarget)
{
	(void)requestTarget;
	return false;
}

/**
 * @brief [TODO:description]
 *
 * @param filePath [TODO:parameter]
 */
void ExecutionHandler::deleteFile(const std::string& filePath)
{
	(void)filePath;
}

/**
 * @brief [TODO:description]
 *
 * @param dirPath [TODO:parameter]
 */
void ExecutionHandler::deleteDirectory(const std::string& dirPath)
{
	(void)dirPath;
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
