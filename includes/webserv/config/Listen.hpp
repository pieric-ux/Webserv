/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CONFIG_LISTEN_HPP
#define WEBSERV_CONFIG_LISTEN_HPP

/**
 * @file Listen.hpp
 * @brief Configuration data for a single server listen directive (bind address, port and per-socket options).
 */

#include <string>
#include <webserv/types.hpp>

namespace webserv
{
namespace config
{

struct Listen
{
	std::string	address;
	std::string	port;
	bool		defaultServer;
	int			backlog;
	int			rcvbuf;
	int			sndbuf;
	bool		ipv6only;
	bool		reuseport;
	bool		so_keepalive;

	Listen();
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_LISTEN_HPP
