/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Server.cpp
 * @brief Implements webserv::Server, which owns a ServerConfig and resolves,
 * creates, binds and listens on the TCP sockets described by its listen
 * directives.
 */

#include <webserv/Server.hpp>

namespace webserv
{

/**
 * @brief Default-constructs a Server with an empty config and no sockets,
 * acquiring the "webserv.server" logger and setting it to DEBUG level.
 */
Server::Server()
	:	_config(),
		_sockets()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.server");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Server instance created with default constructor");
}

/**
 * @brief Constructs a Server from the given configuration, acquiring the
 * "webserv.server" logger and immediately creating its listening sockets.
 *
 * @param config The server configuration whose listen directives drive socket
 * creation.
 */
Server::Server(const config::ServerConfig &config)
	:	_config(config)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.server");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Server instance created with ServerConfig");

	createSockets();
}

/**
 * @brief Destroys the Server; owned sockets are released by their RAII members.
 */
Server::~Server() {}

/**
 * @brief Copy-constructs a Server, copying the logger, configuration and the
 * collection of listening sockets from another instance.
 *
 * @param rhs The Server to copy from.
 */
Server::Server(const Server &rhs)
	:	_logger(rhs._logger),
		_config(rhs._config),
		_sockets(rhs._sockets)
{}

/**
 * @brief Copy-assigns another Server, replacing the logger, configuration and
 * listening sockets while guarding against self-assignment.
 *
 * @param rhs The Server to assign from.
 * @return A reference to this Server.
 */
Server &Server::operator=(const Server &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_config = rhs._config;
		_sockets = rhs._sockets;
	}
	return (*this);
}

/**
 * @brief Returns the shared "webserv.server" logger used by this class.
 *
 * @return The logger registered under the "webserv.server" name.
 */
t_Logger	Server::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.server");
}

/**
 * @brief Provides read-only access to the server's listening sockets.
 *
 * @return A const reference to the collection of bound, listening sockets.
 */
const t_ServerSockets	&Server::getSockets() const
{
	return _sockets;
}

/**
 * @brief Provides read-only access to the server's configuration.
 *
 * @return A const reference to the ServerConfig owned by this Server.
 */
const config::ServerConfig	&Server::getConfig() const
{
	return _config;
}

/**
 * @brief Creates the listening sockets for each listen directive in the config.
 *
 * For every listen entry it resolves candidate addresses (treating "*" as a
 * wildcard), skips IPv4 results when ipv6only is set and any non-IP families,
 * then for each usable address creates a TCP server socket, applies its socket
 * options, binds it, and listens with the configured backlog (falling back to
 * SOMAXCONN when the backlog is unset or exceeds it). Successful sockets are
 * stored in _sockets; per-address and per-listen failures are logged and
 * skipped rather than aborting the whole process.
 */
void	Server::createSockets()
{
	t_Listen listen = _config.getListen();
	t_Listen::const_iterator it = listen.begin();

	for(; it != listen.end(); ++it)
	{
		try{
			std::string address = it->address;
			common::core::net::GetAddrinfo addrinfo(it->address == "*" ? NULL : it->address.c_str(), it->port.c_str(), AI_PASSIVE | AI_NUMERICSERV, AF_UNSPEC, SOCK_STREAM);

			t_AddrPortPair addr;
			struct sockaddr_storage storage;
			struct addrinfo *current = addrinfo.getRes();
			for(; current != NULL; current = current->ai_next)
			{
				if (it->ipv6only && current->ai_family == AF_INET)
				{
	 				ERROR(_logger, "IPv6 only is set but got IPv4 address for " + it->address + ":" + it->port);
					continue;
				}
				if (current->ai_family != AF_INET && current->ai_family != AF_INET6)
				{
					ERROR(_logger, "Unsupported address family for " + it->address + ":" + it->port);
					continue;
				}

				try{
					std::memset(&storage, 0, sizeof(storage));
					std::memmove(&storage, current->ai_addr, current->ai_addrlen);
					addr = common::core::net::getNameInfo(storage);
					DEBUG(_logger, "Resolved address for " + it->address + ":" + it->port + " - " + addr.first + ":" + addr.second);
				} catch (const std::exception &e)
				{
					WARNING(_logger, "Failed to get socket name info: " + std::string(e.what()));
				}

				try{
					common::core::net::TcpServer socket(current->ai_family, current->ai_protocol, true);
					setSocketOption(socket, *it, current->ai_family, addr);

					INFO(_logger, "Created socket for " + addr.first + ":" + addr.second);

					socket.bind(current->ai_addr, current->ai_addrlen);
					INFO(_logger, "Bound socket for " + addr.first + ":" + addr.second);

					if (it->backlog < 0 || it->backlog > SOMAXCONN)
					{
						if (it->backlog < 0)
							WARNING(_logger, "Backlog value " + common::core::utils::toString(it->backlog) + " is unset, using SOMAXCONN instead for " + addr.first + ":" + addr.second);
						else
							WARNING(_logger, "Backlog value " + common::core::utils::toString(it->backlog) + " exceeds SOMAXCONN, using SOMAXCONN instead for " + addr.first + ":" + addr.second);

						socket.listen(SOMAXCONN);
						_sockets.push_back(t_SocketPairServer(socket, storage));
						INFO(_logger, "Listening on " + addr.first + ":" + addr.second + " with backlog " + common::core::utils::toString(SOMAXCONN));
					}
					else
					{
						socket.listen(it->backlog);
						_sockets.push_back(t_SocketPairServer(socket, storage));
						INFO(_logger, "Listening on " + addr.first + ":" + addr.second + " with backlog " + common::core::utils::toString(SOMAXCONN));
					}
				} catch (const std::exception &e)
				{
					ERROR(_logger, "Failed to create socket for " + addr.first + ":" + addr.second + " - " + e.what());
				}
			}
		} catch (const std::exception &e)
		{
			ERROR(_logger, e.what());
			continue;
		}
	}
}

/**
 * @brief Applies the socket-level options requested by a listen directive.
 *
 * Always sets SO_REUSEADDR, and conditionally sets SO_REUSEPORT, SO_KEEPALIVE,
 * SO_RCVBUF and SO_SNDBUF based on the listen settings; for IPv6 sockets it
 * also enables IPV6_V6ONLY. Each applied option is logged at DEBUG level.
 *
 * @param socket The TCP server socket to configure.
 * @param listen The listen directive holding the requested option values.
 * @param ai_family The socket's address family (e.g. AF_INET or AF_INET6),
 * used to decide whether IPV6_V6ONLY is applied.
 * @param addr The resolved address/port pair, used only for log messages.
 */
void	Server::setSocketOption(common::core::net::TcpServer &socket, const config::Listen &listen, const int ai_family, const t_AddrPortPair &addr)
{
	socket.setsockopt<int>(SO_REUSEADDR, 1);
	DEBUG(_logger, "Set SO_REUSEADDR for " + addr.first + ":" + addr.second);
	if (listen.reuseport)
	{
		socket.setsockopt<int>(SO_REUSEPORT, listen.reuseport);
		DEBUG(_logger, "Set SO_REUSEPORT for " + addr.first + ":" + addr.second);
	}
	if (listen.so_keepalive)
	{
		socket.setsockopt<int>(SO_KEEPALIVE, listen.so_keepalive);
		DEBUG(_logger, "Set SO_KEEPALIVE for " + addr.first + ":" + addr.second);
	}
	if (listen.rcvbuf > 0)
	{
		socket.setsockopt<int>(SO_RCVBUF, listen.rcvbuf);
		DEBUG(_logger, "Set SO_RCVBUF for " + addr.first + ":" + addr.second);
	}
	if (listen.sndbuf > 0)
	{
		socket.setsockopt<int>(SO_SNDBUF, listen.sndbuf);
		DEBUG(_logger, "Set SO_SNDBUF for " + addr.first + ":" + addr.second);
	}
	if (ai_family == AF_INET6)
	{
		socket.setsockopt<int>(IPV6_V6ONLY, 1, IPPROTO_IPV6);
		DEBUG(_logger, "Set IPV6_V6ONLY for " + addr.first + ":" + addr.second);
	}
}

} // !webserv
