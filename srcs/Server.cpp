// TODO: don't forget header

/**
 * @file Server.cpp
 * @brief [TODO:description]
 */

#include <webserv/Server.hpp>

namespace webserv
{

/**
 * @brief [TODO:description]
 */
Server::Server()
	:	_config(),
		_sockets()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.server");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "Server instance created with default constructor");
}

/**
 * @brief [TODO:description]
 *
 * @param config [TODO:parameter]
 */
Server::Server(const config::ServerConfig &config)
	:	_config(config)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.server");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "Server instance created with ServerConfig");

	createSockets();
}

/**
 * @brief [TODO:description]
 */
Server::~Server() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Server::Server(const Server &rhs)
	:	_logger(rhs._logger),
		_config(rhs._config),
		_sockets(rhs._sockets)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
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
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	Server::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.server");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_ServerSockets	&Server::getSockets() const
{
	return _sockets;
}

/**
 * @brief [TODO:description]
 */
void	Server::createSockets()
{
	t_Listen listen = _config.getListen();
	t_Listen::const_iterator it = listen.begin();

	for(; it != listen.end(); ++it)
	{
		try{
			common::core::net::Addrinfo addrinfo(it->address.c_str(), it->port.c_str(), AI_PASSIVE | AI_NUMERICSERV, AF_UNSPEC, SOCK_STREAM);

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
					common::core::net::TcpServer socket(current->ai_family, current->ai_protocol, true);
					setSocketOption(socket, *it);
					INFO(_logger, "Created socket for " + it->address + ":" + it->port);
					socket.bind(current->ai_addr, current->ai_addrlen);
					INFO(_logger, "Bound socket for " + it->address + ":" + it->port);
					if (it->backlog < 0 || it->backlog > SOMAXCONN)
					{
						if (it->backlog < 0)
							WARNING(_logger, "Backlog value is not set (-1), using SOMAXCONN instead for " + it->address + ":" + it->port);
						else
							WARNING(_logger, "Backlog value " + common::core::utils::toString(it->backlog) + " exceeds SOMAXCONN, using SOMAXCONN instead for " + it->address + ":" + it->port);
						socket.listen(SOMAXCONN);
						_sockets.push_back(t_SocketPair(addrinfo, socket));
						INFO(_logger, "Listening on socket for " + it->address + ":" + it->port + " with backlog " + common::core::utils::toString(SOMAXCONN));
					}
					else
					{
						socket.listen(it->backlog);
						_sockets.push_back(t_SocketPair(addrinfo, socket));
						INFO(_logger, "Listening on socket for " + it->address + ":" + it->port + " with backlog " + common::core::utils::toString(it->backlog));
					}
				} catch (const std::exception &e)
				{
					ERROR(_logger, "Failed to create socket for " + it->address + ":" + it->port + " - " + e.what());
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
 * @brief [TODO:description]
 *
 * @param socket [TODO:parameter]
 * @param listen [TODO:parameter]
 */
void	Server::setSocketOption(common::core::net::TcpServer &socket, const config::Listen &listen)
{
	socket.setsockopt<int>(SO_REUSEADDR, 1);
	DEBUG(_logger, "Set SO_REUSEADDR for " + listen.address + ":" + listen.port);
	if (listen.reuseport)
	{
		socket.setsockopt<bool>(SO_REUSEPORT, listen.reuseport);
		DEBUG(_logger, "Set SO_REUSEPORT for " + listen.address + ":" + listen.port);
	}
	if (listen.so_keepalive)
	{
		socket.setsockopt<bool>(SO_KEEPALIVE, listen.so_keepalive);
		DEBUG(_logger, "Set SO_KEEPALIVE for " + listen.address + ":" + listen.port);
	}
	if (listen.rcvbuf > 0)
	{
		socket.setsockopt<int>(SO_RCVBUF, listen.rcvbuf);
		DEBUG(_logger, "Set SO_RCVBUF for " + listen.address + ":" + listen.port);
	}
	if (listen.sndbuf > 0)
	{
		socket.setsockopt<int>(SO_SNDBUF, listen.sndbuf);
		DEBUG(_logger, "Set SO_SNDBUF for " + listen.address + ":" + listen.port);
	}
}

} // !webserv
