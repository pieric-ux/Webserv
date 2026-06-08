// TODO: don't forget header

#ifndef WEBSERV_HEADERS_HTTPHEADERS_HPP
#define WEBSERV_HEADERS_HTTPHEADERS_HPP

/**
* @file HTTPHeaders.hpp
* @brief [TODO:description]
*/

#include <string>
#include <webserv/types.hpp>
namespace webserv
{
namespace HTTPheaders
{

class HTTPHeader
{
	public:
		HTTPHeader();
		HTTPHeader(const std::string name, const std::string description);
		HTTPHeader(const std::string name, const std::string value, const std::string description);
		~HTTPHeader();

		HTTPHeader(const HTTPHeader &rhs);
		HTTPHeader &operator=(const HTTPHeader &rhs);

		static t_Logger			getLogger();

		const std::string		&getName() const;
		const std::string		&getValue() const;
		void					setValue(const std::string value);
		const std::string		&getDescription() const;

	private:
		t_Logger				_logger;
		std::string				_name;
		std::string				_value;
		std::string				_description;
};

} // !HTTPheaders
} // !webserv

#endif // !WEBSERV_HEADERS_HTTPHEADERS_HPP
