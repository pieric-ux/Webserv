// TODO: don't forget header

#ifndef WEBSERV_CLIENT_HTTPERROR_HPP
#define WEBSERV_CLIENT_HTTPERROR_HPP

/**
 * @file HTTPError.hpp
 * @brief [TODO:description]
 */

#include <stdexcept>
#include <string>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace client
{

class HTTPError : public std::exception
{
	public:
		HTTPError();
		explicit HTTPError(const status::StatusCode &statusCode);
		virtual ~HTTPError() throw();

		HTTPError(const HTTPError &rhs);
		HTTPError &operator=(const HTTPError &rhs);

		status::StatusCode		getStatusCode() const;
		void					setStatusCode(const status::StatusCode &statusCode);
		const char				*what() const throw();

	private:
		status::StatusCode		_statusCode;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_HTTPERROR_HPP
