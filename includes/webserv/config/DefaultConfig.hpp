// TODO: don't forget header

#ifndef WEBSERV_CONFIG_DEFAULTCONFIG_HPP
#define WEBSERV_CONFIG_DEFAULTCONFIG_HPP

#include <string>
#include <webserv/types.hpp>

#ifndef DEV_LOG
# define DEV_LOG 0
#endif

#ifndef DEFAULT_CONFIG_PATH
# define DEFAULT_CONFIG_PATH "configs/default.conf"
#endif

#ifndef DEFAULT_IOMULTIPLEXER
# define DEFAULT_IOMULTIPLEXER "poll"
#endif

namespace webserv
{
namespace config
{

struct DefaultConfig
{
	// HTTPSERVER
	static const std::string		defaultConfigPath;

	// HTTPCONFIG, SERVERCONFIG, LOCATIONCONFIG
	static const std::string		ioMultiplexer;
	static const bool				autoindex;
	static const int				clientMaxBodySize;
	static const bool				createFullPutPath;
	static const std::string		davPutPath;
	static const std::string		davAccess;
	static const t_DavMethods		davMethods;
	static const std::string		defaultType;
	static const t_ErrorPages		errorPage;
	static const t_Index			index;
	static const int				keepAliveTimeout;
	static const t_AllowedMethods	allowedMethods;
	static const t_Listen			listen;
	static const std::string		root;
	static const t_Servernames		servernames;
	static const t_MimeTypes		types;
	static const bool				enableCGI;
	static const t_CgiExtensions	cgiExtensions;

	// LISTEN
	static const std::string		address;
	static const std::string		port;
	static const bool				defaultServer;
	static const int				backlog;
	static const int				rcvbuf;
	static const int				sndbuf;
	static const bool				ipv6only;
	static const bool				reuseport;
	static const bool				so_keepalive;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_DEFAULTCONFIG_HPP
