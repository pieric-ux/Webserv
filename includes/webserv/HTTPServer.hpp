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
#include <common/core/io/IEventIO.hpp>
#include <log42/Logger.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/handler/ClientHandler.hpp>
#include <webserv/headers/HeaderRegistry.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>
#include <webserv/types/TypesRegistry.hpp>
#include <webserv/Server.hpp>
#include <webserv/ServerFactory.hpp>

namespace webserv
{

class HTTPServer
{
	public:
		static HTTPServer			&getInstance();
		bool						running();

	private:
		abnf::Abnf					_abnf;	
		log42::logger::Logger		_logger;
		std::string					_configPath;
		Parser						_parser;
		HTTPConfig					&_httpConfig;
		ServerFactory				_ServerFactory;
		std::vector<Server>			_Servers;
		common::core::io::IEventIO	_ioMultiplexer;
		ClientHandler				_clientHandler;
		StatusCodeRegistry			&_statusCodeRegistry;
		TypesRegistry				&_typesRegistry;
		HeaderRegistry				&_headerRegistry;

		HTTPConfig					&loadConfig();
		void						connectClient();

		HTTPServer();
		~HTTPServer();
		HTTPServer(const HTTPServer &rhs);
		HTTPServer &operator=(const HTTPServer &rhs);
};

} // !webserv

#endif // !WEBSERV_HTTPSERVER_HPP
