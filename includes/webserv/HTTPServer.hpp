// TODO: don't forget header

#ifndef WEBSERV_HTTPSERVER_HPP
#define WEBSERV_HTTPSERVER_HPP

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

#include <cstdlib>
#include <string>
#include <abnf/Abnf.hpp>
#include <log42/Log42.hpp>
#include <common/common.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/handler/ClientHandler.hpp>
#include <webserv/headers/HTTPHeadersRegistry.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>
#include <webserv/MIMEtypes/TypesRegistry.hpp>
#include <webserv/Server.hpp>
#include <webserv/ServerFactory.hpp>
#include <webserv/types.hpp>
#include <webserv/logging.hpp>

namespace webserv
{

class HTTPServer
{
	public:
		static HTTPServer					&getInstance();

		t_Logger							getLogger() const;

		bool								running();
		void								loadConfig();
		void								connectClient();

		const config::HTTPConfig			&getHTTPConfig() const;
		void								setHTTPconfig(const config::HTTPConfig &httpConfig);
		const ServerFactory					&getServerFactory() const;
		const t_Servers						&getServers() const;
		void								setServers(const t_Servers &servers);
		const t_ioMultiplexer				&getIOMultiplexer() const;
		
	private :
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

		HTTPServer();
		~HTTPServer();
		HTTPServer(const HTTPServer &rhs);
		HTTPServer &operator=(const HTTPServer &rhs);
};

} // !webserv

#endif // !WEBSERV_HTTPSERVER_HPP
