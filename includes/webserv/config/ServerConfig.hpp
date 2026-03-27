// TODO: don't forget header

#ifndef WEBSERV_CONFIG_SERVERCONFIG_HPP
#define WEBSERV_CONFIG_SERVERCONFIG_HPP

/**
 * @file ServerConfig.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/config/method.hpp>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/Listen.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/config/DefaultConfig.hpp>

namespace webserv
{
namespace config
{

class ServerConfig
{
	public:
		ServerConfig();
		~ServerConfig();

		ServerConfig(const ServerConfig &rhs);
		ServerConfig &operator=(const ServerConfig &rhs);

		t_Logger			getLogger() const;

		int					getClientMaxBodySize() const;

		bool				getCreateFullPutPath() const;
		std::string			getDavPutPath() const;
		std::string			getDavAccess() const;
		t_DavMethods		getDavMethods() const;

		std::string			getDefaultType() const;
		t_ErrorPages		getErrorPage() const;
		int					getKeepAliveTimeout() const;
		t_Listen			getListen() const;
		t_LocationConfigs	getLocationConfigs() const;
		std::string			getRoot() const;
		t_Servernames		getServerName() const;
		t_MimeTypes			getTypes() const;
		bool				getEnableCGI() const;
		t_CgiExtensions		getCgiExtensions() const;
		LocationConfig		findLocationConfig(const std::string &requestTarget);

	private:
		t_Logger			_logger;
		int					_clientMaxBodySize;
		bool				_createFullPutPath;
		std::string			_davPutPath;
		std::string			_davAccess;
		t_DavMethods		_davMethods;
		std::string			_defaultType;
		t_ErrorPages		_errorPage;
		int					_keepAliveTimeout;
		t_Listen			_listen;
		t_LocationConfigs	_locationConfigs;
		std::string			_root;
		t_Servernames		_serverName;
		t_MimeTypes			_types;
		bool				_enableCGI;
		t_CgiExtensions		_cgiExtensions;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_SERVERCONFIG_HPP
