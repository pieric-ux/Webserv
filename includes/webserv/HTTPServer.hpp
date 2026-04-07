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
#include <webserv/signal.hpp>

namespace webserv
{

class HTTPServer
{
	public:
		static HTTPServer					&getInstance();

		t_Logger							getLogger() const;

		void								setup();
		void								run();

		void								setConfigPath(const std::string &configPath);
		const config::HTTPConfig			&getHTTPConfig() const;
		void								setHTTPconfig(const config::HTTPConfig &httpConfig);
		ServerFactory						&getServerFactory() const;
		void								setServerFactory(const ServerFactory &serverFactory);
		t_Servers							getServers() const;
		void								addServer(const Server &server);
		t_ioMultiplexer						getIOMultiplexer() const;
		void								setIOMultiplexer(const t_ioMultiplexer &ioMultiplexer);
		
		void								setParser(const parser::Parser &parser);
		void								setAbnf(const abnf::Abnf &abnf);


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
		void								loadConfig();
		void								connectClient(const t_SocketPairServer &socket);

		HTTPServer();
		~HTTPServer();
		HTTPServer(const HTTPServer &rhs);
		HTTPServer &operator=(const HTTPServer &rhs);
};

} // !webserv

#endif // !WEBSERV_HTTPSERVER_HPP
