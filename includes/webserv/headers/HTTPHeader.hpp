// TODO: don't forget header

#ifndef WEBSERV_HEADERS_HTTPHEADERS_HPP
#define WEBSERV_HEADERS_HTTPHEADERS_HPP

/**
* @file HTTPHeaders.hpp
* @brief [TODO:description]
*/

#include <string>

namespace http
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

		std::string getName() const;
		std::string getValue() const;
		void 		setValue(const std::string value);
		std::string getDescription() const;

	private:
		std::string _name;
		std::string _value;
		std::string _description;
};

} // !HTTPheaders
} // !http

#endif // !WEBSERV_HEADERS_HTTPHEADERS_HPP
