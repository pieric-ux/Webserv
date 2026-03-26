// TODO: don't forget header

#ifndef WEBSERV_SERVERFACTORY_HPP
#define WEBSERV_SERVERFACTORY_HPP

/**
 * @file ServerFactory.hpp
 * @brief [TODO:description]
 */

#include <webserv/config/HTTPConfig.hpp>
#include <webserv/types.hpp>

namespace webserv
{

class ServerFactory
{
	public:
		ServerFactory();
		explicit ServerFactory(const config::HTTPConfig &httpConfig);
		~ServerFactory();

		ServerFactory(const ServerFactory &rhs);
		ServerFactory &operator=(const ServerFactory &rhs);

	private:
		const config::HTTPConfig	&_httpConfig;
		t_Logger					_logger;
};

} // !webserv

#endif // !WEBSERV_SERVERFACTORY_HPP
