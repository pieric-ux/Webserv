// TODO: don't forget header

#ifndef WEBSERV_CONFIG_SERVERCONFIG_HPP
#define WEBSERV_CONFIG_SERVERCONFIG_HPP

/**
 * @file ServerConfig.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <map>
#include <string>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/Listen.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/types.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/method.hpp>

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

		int									getClientMaxBodySize() const;

		bool								getCreateFullPutPath() const;
		std::string							getDavPutPath() const;
		std::string							getDavAccess() const;
		std::vector<config::e_Method>		getDavMethods() const;

		std::string							getDefaultType() const;
		std::vector<ErrorPage>				getErrorPage() const;
		int									getKeepAliveTimeout() const;
		std::vector<Listen>					getListen() const;
		std::vector<LocationConfig>			getLocationConfigs() const;
		std::string							getRoot() const;
		std::vector<std::string>			getServerName() const;
		std::map<std::string, std::string>	getTypes() const;
		bool								getEnableCGI() const;
		std::map<std::string, std::string>	getCgiExtensions() const;
		LocationConfig						findLocationConfig(const std::string &requestTarget);

	private:
		t_Logger							_logger;
		int									_clientMaxBodySize;
		bool								_createFullPutPath;
		std::string							_davPutPath;
		std::string							_davAccess;
		std::vector<config::e_Method>		_davMethods;
		std::string							_defaultType;
		std::vector<ErrorPage>				_errorPage;
		int									_keepAliveTimeout;
		std::vector<Listen>					_listen;
		std::vector<LocationConfig>			_locationConfigs;
		std::string							_root;
		std::vector<std::string>			_serverName;
		std::map<std::string, std::string>	_types;
		bool								_enableCGI;
		std::map<std::string, std::string>	_cgiExtensions;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_SERVERCONFIG_HPP
