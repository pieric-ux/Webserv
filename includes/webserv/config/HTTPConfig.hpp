// TODO: don't forget header

#ifndef WEBSERV_CONFIG_HTTPCONFIG_HPP
#define WEBSERV_CONFIG_HTTPCONFIG_HPP

/**
 * @file HTTPConfig.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <map>
#include <string>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/ServerConfig.hpp>

namespace webserv
{
namespace config
{

enum e_Method
{
	GET,
	HEADER,
	POST,
	PUT,
	DELETE
};

class HTTPConfig
{
	public:
		static HTTPConfig					&getInstance();

		int									getClientMaxBodySize() const;

		bool								getCreateFullPutPath() const;
		std::string							getDavPutPath() const;
		std::string							getDavAccess() const;
		std::vector<config::e_Method>		getDavMethods() const;

		std::string							getDefaultType() const;
		std::vector<ErrorPage>				getErrorPage() const;
		int									getKeepAliveTimeout() const;
		std::string							getRoot() const;
		std::vector<ServerConfig>			getServerConfigs() const;
		std::map<std::string, std::string>	getTypes() const;
		bool								getEnableCGI() const;
		std::map<std::string, std::string>	getCgiExtensions() const;

	private:
		int									_clientMaxBodySize;
		bool								_createFullPutPath;
		std::string							_davPutPath;
		std::string							_davAccess;
		std::vector<config::e_Method>		_davMethods;
		std::string							_defaultType;
		std::vector<ErrorPage>				_errorPage;
		int									_keepAliveTimeout;
		std::string							_root;
		std::vector<ServerConfig>			_serverConfigs;
		std::map<std::string, std::string>	_types;
		bool								_enableCGI;
		std::map<std::string, std::string>	_cgiExtensions;

		HTTPConfig();
		~HTTPConfig();
		HTTPConfig(const HTTPConfig &rhs);
		HTTPConfig &operator=(const HTTPConfig &rhs);
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_HTTPCONFIG_HPP
