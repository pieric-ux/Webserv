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
	:	_logger(),
		_socket4(),
		_socket6(),
		_addrinfo4(),
		_addrinfo6(),
		_config(),
		_parser()
{}

/**
 * @brief [TODO:description]
 *
 * @param config [TODO:parameter]
 */
Server::Server(const config::ServerConfig &config)
	:	_logger(),
		_socket4(),
		_socket6(),
		_addrinfo4(),
		_addrinfo6(),
		_config(config),
		_parser()
{}

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
		_socket4(rhs._socket4),
		_socket6(rhs._socket6),
		_addrinfo4(rhs._addrinfo4),
		_addrinfo6(rhs._addrinfo6),
		_config(rhs._config),
		_parser(rhs._parser)
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
		_socket4 = rhs._socket4;
		_socket6 = rhs._socket6;
		_addrinfo4 = rhs._addrinfo4;
		_addrinfo6 = rhs._addrinfo6;
		_config = rhs._config;
		_parser = rhs._parser;
	}
	return (*this);
}

} // !webserv
