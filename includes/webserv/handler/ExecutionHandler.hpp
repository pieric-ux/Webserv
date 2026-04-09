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
#include <webserv/handler/RequestHandler.hpp>
#include <webserv/handler/ResponseHandler.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/types.hpp>

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

		static t_Logger					getLogger();

		int								getFd() const;
		std::size_t						getBodyReceived() const;
		void							setFlags(const int flags);
		void							setBodyReceived(const std::size_t bodyReceived);
		int								getFlags() const;

		void							execute(const RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::ServerConfig &serverConfig);
		void							executeCGI(client::Request &request, const config::ServerConfig &serverConfig);
		void							executeRequest(client::Request &request, client::Response &response, const config::ServerConfig &serverConfig);

	private:	
		t_Logger						_logger;
		common::core::raii::UniqueFd	_fd;
		ssize_t							_bodyReceived;
		e_ExecutionHandlerFlags			_flags;

		void							executeHEADorGET(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig);
		void							executePOST(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig);
		void							executeDELETE(client::Request &request, client::Response &response, const config::LocationConfig &locationConfig);

		void							openFile(const client::Request &request, const config::LocationConfig &locationConfig);
		void							readChunk(handler::ResponseHandler &responseHandler);
		void							writeChunk(handler::RequestHandler &requestHandler);
		int								getFileSize(const std::string &requestTarget);
		std::string						getFileExtension(const std::string &requestTarget);
		bool							isFile(const std::string& requestTarget);
		bool							isFileExisting(const std::string& requestTarget);
		void							deleteFile(const std::string& filePath);
		void							deleteDirectory(const std::string& dirPath);
		bool							isDirectory(const std::string &path);
		std::string						joinPath(const std::string &dir, const std::string &name);
		std::string						generateAutoindexHTML(const std::string &dirPath);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_EXECUTIONHANDLER_HPP
