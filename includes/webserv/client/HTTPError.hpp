// TODO: don't forget header

#ifndef WEBSERV_CLIENT_HTTPERROR_HPP
#define WEBSERV_CLIENT_HTTPERROR_HPP

/**
 * @file HTTPError.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/status/StatusCode.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>

namespace webserv
{
namespace client
{

class HTTPError : public std::exception
{
	public:
		HTTPError();
		explicit HTTPError(unsigned short code);
		virtual ~HTTPError() throw();

		HTTPError(const HTTPError &rhs);
		HTTPError &operator=(const HTTPError &rhs);

		static t_Logger			getLogger();

		status::StatusCode		getStatusCode() const;
		void					setStatusCode(const status::StatusCode &statusCode);
		const char				*what() const throw();

	private:
		t_Logger				_logger;
		status::StatusCode		_statusCode;
		std::string				_what;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_HTTPERROR_HPP
