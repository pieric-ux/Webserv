// TODO: don't forget header

#ifndef WEBSERV_HANDLER_REQUESTHANDLER_HPP
#define WEBSERV_HANDLER_REQUESTHANDLER_HPP

/**
 * @file RequestHandler.hpp
 * @brief [TODO:description]
 */

#include <algorithm>
#include <sstream>
#include <vector>
#include <string>
#include <common/common.hpp>
#include <webserv/client/HTTPError.hpp>
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
		RequestHandler(config::ServerConfig &serverConfig);
		~RequestHandler();

		RequestHandler(const RequestHandler &rhs);
		RequestHandler &operator=(const RequestHandler &rhs);

		static t_Logger			getLogger();

		void					parseHeaders();
		void					parseBody();

		client::Request			&getRequest();
		const client::Request	&getRequest() const;
		void					setRequest(const client::Request &request);
		const t_raw				&getBufferRequest() const;
		void					appendToBufferRequest(const t_raw &buffer);
		void					clearBufferRequest();
		parser::Parser			&getParser();

	private:
		t_Logger				_logger;
		client::Request			_request;
		t_raw					_bufferRequest;
		parser::Parser			_parser;
		config::ServerConfig	&_serverConfig;

		void					validateHeaders();
		void					buildAbsolutPath();
		std::string				normalizePath(const std::string &path);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_REQUESTHANDLER_HPP
