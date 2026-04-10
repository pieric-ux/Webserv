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

		void							execute(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig);
		void							executeCGI(const RequestHandler &requestHandler, const config::LocationConfig &locationConfig);
		void							executeRequest(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig);

	private:
		t_Logger						_logger;
		common::core::raii::UniqueFd	_fd;
		ssize_t							_bodyReceived;
		e_ExecutionHandlerFlags			_flags;
		t_raw							_autoindexBuffer;

		void							executeHEADorGET(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig);
		void							executePOST(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig);
		void							executeDELETE(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig);
		void							executePUT(RequestHandler &requestHandler, ResponseHandler &responseHandler, const config::LocationConfig &locationConfig);

		void							openFile(const RequestHandler &requestHandler, const config::LocationConfig &locationConfig);
		void							readChunk(ResponseHandler &responseHandler);
		void							writeChunk(const RequestHandler &requestHandler);
		int								getFileSize(const std::string &path);
		std::string						getFileExtension(const std::string &path);
		bool							isFile(const std::string& path);
		bool							isExisting(const std::string& path);
		void							deleteFile(const std::string& filePath);
		void							deleteDirectory(const std::string& dirPath);
		bool							isDirectory(const std::string &path);
		std::string						generateAutoindexHTML(const std::string &dirPath);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_EXECUTIONHANDLER_HPP
