// TODO: don't forget header

#ifndef WEBSERV_TYPES_HPP
#define WEBSERV_TYPES_HPP

#include "common/core/net/sockets/TcpServer.hpp"
#include <list>
#include <vector>
#include <log42/Log42.hpp>
#include <common/common.hpp>
#include <webserv/config/method.hpp>

namespace webserv
{

class Server;
namespace HTTPheaders
{
	class HTTPHeader;
	class HTTPHeadersRegistry;
}
namespace client { class Client; }
namespace config
{
	class ErrorPage;
	struct Listen;
	class LocationConfig;
	class ServerConfig;
}
namespace status { class StatusCode; }

/**
 * @typedef t_Logger
 * @brief Shared pointer to a logger instance.
 */
typedef common::core::raii::SharedPtr<log42::logger::Logger>								t_Logger;

typedef common::core::raii::SharedPtr<common::core::io::IEventIO>							t_ioMultiplexer;

typedef std::vector<std::list<HTTPheaders::HTTPHeader> >									t_Headers;

typedef std::map<std::string, std::string>													t_MimeTypes;

typedef std::map<std::string, std::string>													t_CgiExtensions;

typedef std::vector<config::e_Method>														t_DavMethods;

typedef std::vector<config::e_Method>														t_AllowedMethods;

typedef std::vector<config::ErrorPage>														t_ErrorPages;

typedef std::vector<config::ServerConfig>													t_ServerConfigs;

typedef std::vector<unsigned char>															t_raw;

typedef std::vector<status::StatusCode>														t_StatusCodes;

typedef std::vector<std::string>															t_Index;

typedef std::vector<config::Listen>															t_Listen;

typedef std::vector<config::LocationConfig>													t_LocationConfigs;

typedef std::vector<std::string>															t_Servernames;

typedef std::map<int, client::Client>														t_Clients;

typedef std::vector<Server>																	t_Servers;

typedef std::map<std::string, HTTPheaders::HTTPHeader>										t_HeadersRegistry;

typedef std::map<unsigned short, status::StatusCode>										t_StatusCodesRegistry;

typedef std::pair<common::core::net::Addrinfo, common::core::net::TcpServer>				t_SocketPair;

typedef std::vector<t_SocketPair>															t_ServerSockets;

typedef std::vector<std::string>															t_SubRules;

typedef unsigned int																		t_clientMaxBodySize;

typedef unsigned int																		t_keepAliveTimeout;

typedef unsigned short												t_Perms;
} // !webserv

#endif // !WEBSERV_TYPES_HPP
