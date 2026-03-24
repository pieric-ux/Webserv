// TODO: don't forget header

#ifndef WEBSERV_SERVERFACTORY_HPP
#define WEBSERV_SERVERFACTORY_HPP

/**
 * @file ServerFactory.hpp
 * @brief [TODO:description]
 */

#include <webserv/config/HTTPConfig.hpp>

namespace webserv
{

class ServerFactory
{
	public:
		ServerFactory(HTTPConfig &httpConfig);
		~ServerFactory();

		ServerFactory(const ServerFactory &rhs);
		ServerFactory &operator=(const ServerFactory &rhs);

	private:
		HTTPConfig	&_httpConfig;
};

} // !webserv

#endif // !WEBSERV_SERVERFACTORY_HPP
