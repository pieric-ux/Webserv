// TODO: don't forget header

#include <webserv/config/method.hpp>
#include <webserv/config/DefaultConfig.hpp>
#include <webserv/config/Listen.hpp>
#include <webserv/config/ErrorPage.hpp>


namespace {

webserv::t_AllowedMethods initAllowedMethods()
{
	webserv::t_AllowedMethods allowedMethods;
	allowedMethods.insert(webserv::config::GET);
	allowedMethods.insert(webserv::config::POST);
	allowedMethods.insert(webserv::config::DELETE);
	return allowedMethods;
}

webserv::t_MimeTypes initTypes()
{
	webserv::t_MimeTypes types;
	types["html"] = "text/html";
	types["gif"] = "image/gif";
	types["jpg"] = "image/jpeg";
	return types;
}

webserv::t_CgiExtensions initCgiExtensions()
{
	webserv::t_CgiExtensions cgiExtensions;
	cgiExtensions[".py"] = "/bin/python3";
	cgiExtensions[".sh"] = "/bin/bash";
	return cgiExtensions;
}

} // !

namespace webserv
{
namespace config
{

// HTTPSERVER
const std::string			DefaultConfig::defaultConfigPath = DEFAULT_CONFIG_PATH;
const std::size_t			DefaultConfig::URI_MAX_LENGTH;
const std::size_t			DefaultConfig::BUFFER_SIZE;
const std::size_t			DefaultConfig::CLIENT_HEADER_BUFFER_SIZE;

// LISTEN
const std::string			DefaultConfig::address = "*";
const std::string			DefaultConfig::port = "80";
const bool					DefaultConfig::defaultServer = false;
const int					DefaultConfig::backlog = -1;
const int					DefaultConfig::rcvbuf = -1;
const int					DefaultConfig::sndbuf = -1;
const int					DefaultConfig::ipv6only = false;
const int					DefaultConfig::reuseport = false;
const int					DefaultConfig::so_keepalive = false;

// HTTPCONFIG, SERVERCONFIG, LOCATIONCONFIG
const std::string			DefaultConfig::ioMultiplexer = DEFAULT_IOMULTIPLEXER;
const bool					DefaultConfig::autoindex = false;
const t_clientMaxBodySize	DefaultConfig::clientMaxBodySize = 1048576;
const std::string			DefaultConfig::davPutPath = "";
const t_Perms				DefaultConfig::davAccess = 0600;
const t_DavMethods			DefaultConfig::davMethods = t_DavMethods();
const std::string			DefaultConfig::defaultType = "text/plain";
const t_ErrorPages			DefaultConfig::errorPage = t_ErrorPages();
const t_Index				DefaultConfig::index = t_Index(1, "index.html");
const t_keepAliveTimeout	DefaultConfig::keepAliveTimeout = 75;
const t_sessionTTL			DefaultConfig::sessionTTL = SESSION_TTL_S;
const t_AllowedMethods		DefaultConfig::allowedMethods = initAllowedMethods();
const t_Listen				DefaultConfig::listen = t_Listen(1, Listen());
const std::string			DefaultConfig::root = "html";
const t_Servernames			DefaultConfig::servernames = t_Servernames(1, "");
const t_MimeTypes			DefaultConfig::types = initTypes();
const bool					DefaultConfig::enableCGI = false;
const t_CgiExtensions		DefaultConfig::cgiExtensions = initCgiExtensions();

} // !config
} // !webserv
