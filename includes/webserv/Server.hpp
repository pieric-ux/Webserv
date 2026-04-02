// TODO: don't forget header

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

/**
 * @file Server.hpp
 * @brief [TODO:description]
 */

#include <webserv/types.hpp>
#include <common/common.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/parser/Parser.hpp>

namespace webserv
{

class Server
{
	public:
		Server();
		explicit Server(const config::ServerConfig &config);
		~Server();

		Server(const Server &rhs);
		Server &operator=(const Server &rhs);

		static t_Logger					getLogger();

		const t_ServerSockets			&getSockets() const;

	private:
		t_Logger						_logger;
		config::ServerConfig			_config;
		t_ServerSockets 				_sockets;

		void							createSockets();
		void							setSocketOption(common::core::net::TcpServer &socket, const config::Listen &listen);
};

} // !webserv

#endif // !WEBSERV_SERVER_HPP
