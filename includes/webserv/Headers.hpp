// TODO: don't forget header

#ifndef WEBSERV_HEADERS_HPP
#define WEBSERV_HEADERS_HPP

/**
* @file Headers.hpp
* @brief [TODO:description]
*/

#include <string>

namespace http
{
namespace headers
{

class Headers
{
	public:
		Headers();
		Headers(const std::string name, const std::string description);
		~Headers();

		Headers(const Headers &rhs);
		Headers &operator=(const Headers &rhs);

		std::string getName() const;
		std::string getDescription() const;

	private:
		std::string _name;
		std::string _description;
};

} // !headers
} // !http

#endif

