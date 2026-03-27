// TODO: don't forget header

#ifndef WEBSERV_HTTPSERVER_HPP
#define WEBSERV_HTTPSERVER_HPP

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

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

namespace webserv
{

class HTTPServer
{
	public:
		static HTTPServer					&getInstance();

		t_Logger							getLogger() const;

		bool								running();

		const config::HTTPConfig			&getHTTPConfig() const;
		void								setHTTPconfig(const config::HTTPConfig &httpConfig);
		ServerFactory						&getServerFactory() const;
		void								setServerFactory(const ServerFactory &serverFactory);
		t_Servers							getServers() const;
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
		t_Servers							_servers;
		t_ioMultiplexer						_ioMultiplexer;
		handler::ClientHandler				_clientHandler;
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
