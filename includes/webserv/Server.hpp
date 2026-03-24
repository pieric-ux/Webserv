// TODO: don't forget header

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

/**
 * @file Server.hpp
 * @brief [TODO:description]
 */

#include <common/core/net/sockets/Addrinfo.hpp>
#include <common/core/net/sockets/TcpServer.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/parser/Parser.hpp>

namespace webserv
{

class Server
{
	public:
		Server(const ServerConfig &config);
		~Server();

		Server(const Server &rhs);
		Server &operator=(const Server &rhs);

	private:
		common::core::net::TcpServer	_socket4;
		common::core::net::TcpServer	_socket6;
		common::core::net::Addrinfo		_addrinfo4;
		common::core::net::Addrinfo		_addrinfo6;
		ServerConfig					_config;
		Parser							_parser;
};

} // !webserv

#endif // !WEBSERV_SERVER_HPP
