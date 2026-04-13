// TODO: don't forget header

#ifndef WEBSERV_SERVERFACTORY_HPP
#define WEBSERV_SERVERFACTORY_HPP

/**
 * @file ServerFactory.hpp
 * @brief [TODO:description]
 */

#include <webserv/config/HTTPConfig.hpp>
#include <webserv/Server.hpp>
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

		static t_Logger				getLogger();

		t_Servers					createServers() const;

	private:
		t_Logger					_logger;
		const config::HTTPConfig	&_httpConfig;
};

} // !webserv

#endif // !WEBSERV_SERVERFACTORY_HPP
