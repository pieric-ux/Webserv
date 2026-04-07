// TODO: don't forget header

#ifndef WEBSERV_HANDLER_REQUESTHANDLER_HPP
#define WEBSERV_HANDLER_REQUESTHANDLER_HPP

/**
 * @file RequestHandler.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/client/Request.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace handler
{

class RequestHandler
{
	public:
		RequestHandler();
		~RequestHandler();

		RequestHandler(const RequestHandler &rhs);
		RequestHandler &operator=(const RequestHandler &rhs);

		static t_Logger		getLogger();

		client::Request		&getRequest();
		void				setRequest(const client::Request &request);
		const t_raw			&getBufferRequest() const;
		void				appendToBufferRequest(const t_raw &buffer);
		void				clearBufferRequest();
		parser::Parser		&getParser();
		int					getBodyReceived() const;

	private:
		t_Logger			_logger;
		client::Request		_request;
		t_raw				_bufferRequest;
		parser::Parser		_parser;

		void				parseHeadersFromBufferRequest();
		void				validateHeaders(const config::ServerConfig &serverConfig);
		void				parseBodyFromBuffer(const config::ServerConfig &serverConfig);
		void				buildAbsolutPath(const std::string &requestTarget, const config::LocationConfig &locationConfig);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_REQUESTHANDLER_HPP
