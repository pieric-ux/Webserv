// TODO: don't forget header

#ifndef WEBSERV_HANDLER_REQUESTHANDLER_HPP
#define WEBSERV_HANDLER_REQUESTHANDLER_HPP

/**
 * @file RequestHandler.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <string>
#include <webserv/client/Request.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/parser/Parser.hpp>

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

		Request						&getRequest();
		void						setRequest(const Request &request);
		void						appendToBufferRequest(const std::string &buffer);
		void						clearBufferRequest();
		Parser						&getParser();
		int							getBodyReceived() const;

	private:
		Request						_request;
		std::vector<unsigned char>	_bufferRequest;
		Parser						_parser;

		void						parseHeadersFromBufferRequest();
		void						validateHeaders(const ServerConfig &serverConfig);
		void						parseBodyFromBuffer(const ServerConfig &serverConfig);
		void						buildAbsolutPath(const std::string &requestTarget, const LocationConfig &locationConfig);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_REQUESTHANDLER_HPP
