// TODO: don't forget header

#include <webserv/config/Listen.hpp>
#include <webserv/config/DefaultConfig.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief [TODO:description]
 */
Listen::Listen()
	:	address(DefaultConfig::address),
		port(DefaultConfig::port),
		defaultServer(DefaultConfig::defaultServer),
		backlog(DefaultConfig::backlog),
		rcvbuf(DefaultConfig::rcvbuf),
		sndbuf(DefaultConfig::sndbuf),
		doBind(DefaultConfig::doBind),
		ipv6only(DefaultConfig::ipv6only),
		reuseport(DefaultConfig::reuseport),
		so_keepalive(DefaultConfig::so_keepalive)
{}

} // !config
} // !webserv
