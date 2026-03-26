// TODO: don't forget header

#ifndef WEBSERV_HTTPSERVER_HPP
#define WEBSERV_HTTPSERVER_HPP

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <string>
#include <abnf/Abnf.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/handler/ClientHandler.hpp>
#include <webserv/headers/HTTPHeadersRegistry.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>
#include <webserv/MIMEtypes/TypesRegistry.hpp>
#include <webserv/Server.hpp>
#include <webserv/ServerFactory.hpp>
#include <webserv/types.hpp>


#ifndef DEFAULT_CONFIG_PATH
# define DEFAULT_CONFIG_PATH "configs/default.conf"
#endif

namespace webserv
{

class HTTPServer
{
	public:
		static HTTPServer					&getInstance();
		bool								running();

		const config::HTTPConfig			&getHTTPConfig() const;
		void								setHTTPconfig(const config::HTTPConfig &httpConfig);
		ServerFactory						&getServerFactory() const;
		void								setServerFactory(const ServerFactory &serverFactory);
		std::vector<Server>					getServers() const;
		void								addServer(const Server &server);
		t_ioMultiplexer						getIOMultiplexer() const;
		void								setIOMultiplexer(const t_ioMultiplexer &ioMultiplexer);
		
	private :
		abnf::Abnf							&_abnf;
		t_Logger							_logger;
		std::string							_defaultConfigPath;
		parser::Parser						_parser;
		const config::HTTPConfig			&_httpConfig;
		ServerFactory						_serverFactory;
		std::vector<Server>					_servers;
		t_ioMultiplexer						_ioMultiplexer;
		webserv::handler::ClientHandler		_clientHandler;
		HTTPheaders::HTTPHeadersRegistry	&_headerRegistry;
		status::StatusCodeRegistry			&_statusCodeRegistry;
		types::TypesRegistry				&_typesRegistry;
		
		config::HTTPConfig					&loadConfig();
		void								connectClient();

		HTTPServer();
		~HTTPServer();
		HTTPServer(const HTTPServer &rhs);
		HTTPServer &operator=(const HTTPServer &rhs);
};

} // !webserv

#endif // !WEBSERV_HTTPSERVER_HPP
