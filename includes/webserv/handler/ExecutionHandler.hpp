// TODO: don't forget header

#ifndef WEBSERV_HANDLER_EXECUTIONHANDLER_HPP
#define WEBSERV_HANDLER_EXECUTIONHANDLER_HPP

/**
 * @file ExecutionHandler.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/client/Request.hpp>
#include <webserv/client/Response.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>

namespace webserv
{
namespace handler
{

enum e_ExecutionHandlerFlags
{
	E_EXEC_FILE_OPENED = 1 << 0,
	E_EXEC_COMPLETE = 1 << 1,
	E_EXEC_CREATED = 1 << 2,
	E_EXEC_NOCONTENT = 1 << 3
};

class ExecutionHandler
{
	public:
		ExecutionHandler();
		~ExecutionHandler();

		ExecutionHandler(const ExecutionHandler &rhs);
		ExecutionHandler &operator=(const ExecutionHandler &rhs);

		int			getFd() const;
		void		setFd(const int fd);
		int			getBodyReceived() const;
		void		setFlags(const int flags);
		void		setBodyReceived(const int bodyReceived);
		int			getFlags() const;

		void		execute(client::Request &request, client::Response &response, const config::ServerConfig &serverConfig);
		void		executeCGI(client::Request &request, const config::ServerConfig &serverConfig);
		void		executeRequest(client::Request &request, client::Response &response, const config::ServerConfig &serverConfig);

	private:
		int							_fd;
		int							_bodyReceived;
		e_ExecutionHandlerFlags		_flags;

		void		executeHEADorGET(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig);
		void		executePOST(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig);
		void		executeDELETE(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig);

		int			openFile(const client::Request &request, const config::LocationConfig &locationConfig);
		void		readChunk(const int fd, client::Response &response);
		void		writeChunk(const int fd, client::Request &request);

		int			getFileSize(const int fd);
		std::string	getFileExtension(const std::string &requestTarget);
		bool		isFile(const std::string& requestTarget);
		bool		isFileExisting(const std::string& requestTarget);
		void		deleteFile(const std::string& filePath);
		void		deleteDirectory(const std::string& dirPath);

		std::string	generateAutoindexHTML(const std::string &dirPath);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_EXECUTIONHANDLER_HPP
