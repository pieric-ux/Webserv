/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <webserv/config/Listen.hpp>
#include <webserv/config/DefaultConfig.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief Constructs a Listen directive with every member initialized to its DefaultConfig value.
 */
Listen::Listen()
	:	address(DefaultConfig::address),
		port(DefaultConfig::port),
		defaultServer(DefaultConfig::defaultServer),
		backlog(DefaultConfig::backlog),
		rcvbuf(DefaultConfig::rcvbuf),
		sndbuf(DefaultConfig::sndbuf),
		ipv6only(DefaultConfig::ipv6only),
		reuseport(DefaultConfig::reuseport),
		so_keepalive(DefaultConfig::so_keepalive)
{}

} // !config
} // !webserv
