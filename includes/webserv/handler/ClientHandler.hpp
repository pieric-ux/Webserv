/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HANDLER_CLIENTHANDLER_HPP
#define WEBSERV_HANDLER_CLIENTHANDLER_HPP

/**
 * @file ClientHandler.hpp
 * @brief Declares the ClientHandler, which tracks connected clients and drives their HTTP processing through the I/O multiplexer.
 */

#include <ctime>
#include <common/common.hpp>
#include <webserv/client/Client.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace handler
{

class ClientHandler
{
	public:
		ClientHandler();
		~ClientHandler();

		ClientHandler(const ClientHandler &rhs);
		ClientHandler &operator=(const ClientHandler &rhs);

		static t_Logger		getLogger();

		void				setIoMultiplexer(const t_ioMultiplexer &ioMultiplexer);

		void				addClient(const t_SocketPairClient &client, const config::ServerConfig &serverConfig);
		t_Clients::iterator	removeClient(client::Client &client);
		void				processClients();

	private:
		t_Logger			_logger;
		t_Clients			_clients;
		t_ioMultiplexer		_ioMultiplexer;
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_CLIENTHANDLER_HPP
