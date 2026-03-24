// TODO: don't forget header

#ifndef WEBSERV_HANDLER_RESPONSEHANDLER_HPP
#define WEBSERV_HANDLER_RESPONSEHANDLER_HPP

/**
 * @file ResponseHandler.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <string>
#include <webserv/client/Response.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace handler
{

class ResponseHandler
{
	public:
		ResponseHandler();
		~ResponseHandler();

		ResponseHandler(const ResponseHandler &rhs);
		ResponseHandler &operator=(const ResponseHandler &rhs);

		Response					&getResponse();
		void						setResponse(const Response &response);
		void						appendToBufferResponse(const std::string &buffer);
		void						clearBufferResponse();

	private:
		Response					_response;
		std::vector<unsigned char>	_bufferResponse;

		void						buildStatusLine(const std::string &httpVersion, const StatusCode &statusCode, const std::string &reasonPhrase);
		void						buildHeaders(const Request &request);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_RESPONSEHANDLER_HPP
