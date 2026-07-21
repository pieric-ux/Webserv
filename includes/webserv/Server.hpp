/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_HPP
#define WEBSERV_SERVER_HPP

/**
 * @file Server.hpp
 * @brief Declares webserv::Server, which owns a ServerConfig and creates,
 * binds and listens on the TCP sockets described by its listen directives.
 */

#include <cstring>
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
		const config::ServerConfig		&getConfig() const;

	private:
		t_Logger						_logger;
		config::ServerConfig			_config;
		t_ServerSockets 				_sockets;

		void							createSockets();
		void							setSocketOption(common::core::net::TcpServer &socket, const config::Listen &listen, const int ai_family, const t_AddrPortPair &addr);
};

} // !webserv

#endif // !WEBSERV_SERVER_HPP
