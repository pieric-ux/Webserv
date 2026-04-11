// TODO: don't forget header

#ifndef WEBSERV_HANDLER_RESPONSEHANDLER_HPP
#define WEBSERV_HANDLER_RESPONSEHANDLER_HPP

/**
 * @file ResponseHandler.hpp
 * @brief [TODO:description]
 */

#include <sstream>
#include <ctime>
#include <string>
#include <webserv/client/Response.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/status/StatusCode.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>
#include <webserv/types.hpp>

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

		static t_Logger			getLogger();

		client::Response		&getResponse();
		const client::Response	&getResponse() const;
		void					setResponse(const client::Response &response);
		void					appendToBufferResponse(const t_raw &buffer);
		void					clearBufferResponse();
		const t_raw				&getBufferResponse() const;
		void					eraseBufferResponseFront(std::size_t n);
		void					buildHeadersResponse(const client::Request &request, int execFlags, int parsFlags);
		void					buildErrorResponse(const client::Request &request, const client::HTTPError &error);

	private:
		t_Logger				_logger;
		client::Response		_response;
		t_raw					_bufferResponse;

		void					buildStatusLine(const std::string &httpVersion, const status::StatusCode &statusCode, const std::string &reasonPhrase);
		void					buildHeaders(const client::Request &request, int parsFlags);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_RESPONSEHANDLER_HPP
