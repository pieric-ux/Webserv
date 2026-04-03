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
 *
 * @return [TODO:return]
 */
const config::ServerConfig	&Server::getConfig() const
{
	return _config;
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
			common::core::net::GetAddrinfo addrinfo(it->address.c_str(), it->port.c_str(), AI_PASSIVE | AI_NUMERICSERV, AF_UNSPEC, SOCK_STREAM);

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
				} catch (const std::exception &e)
				{
					WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
				}

				try{
					common::core::net::TcpServer socket(current->ai_family, current->ai_protocol, true);
					setSocketOption(socket, *it, addr);

					INFO(_logger, "Created socket for " + addr.first + ":" + addr.second);

					socket.bind(current->ai_addr, current->ai_addrlen);
					INFO(_logger, "Bound socket for " + addr.first + ":" + addr.second);

					if (it->backlog < 0 || it->backlog > SOMAXCONN)
					{
						if (it->backlog < 0)
							WARNING(_logger, "Backlog value " + common::core::utils::toString(it->backlog) + " is not set (-1), using SOMAXCONN instead for " + addr.first + ":" + addr.second);
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
 * @brief [TODO:description]
 *
 * @param socket [TODO:parameter]
 * @param listen [TODO:parameter]
 */
void	Server::setSocketOption(common::core::net::TcpServer &socket, const config::Listen &listen, const t_AddrPortPair &addr)
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
}

} // !webserv
