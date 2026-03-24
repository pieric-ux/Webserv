// TODO: don't forget header

#ifndef WEBSERV_CONFIG_LISTEN_HPP
#define WEBSERV_CONFIG_LISTEN_HPP

/**
 * @file Listen.hpp
 * @brief [TODO:description]
 */

#include <string>

namespace webserv
{
namespace config
{

struct Listen
{
	std::string	address;
	int			port;
	bool		defaultServer;
	int			backlog;
	int			rcvbuf;
	int			sndbuf;
	bool		bind;
	bool		ipv6only;
	bool		reuseport;
	bool		so_keepalive;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_LISTEN_HPP
