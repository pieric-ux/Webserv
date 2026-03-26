// TODO: don't forget header

#ifndef WEBSERV_CONFIG_HTTPCONFIG_HPP
#define WEBSERV_CONFIG_HTTPCONFIG_HPP

/**
 * @file HTTPConfig.hpp
 * @brief [TODO:description]
 */

#include <map>
#include <string>
#include <vector>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/method.hpp>
#include <webserv/types.hpp>

#ifndef DEFAULT_CLIENT_MAX_BODY_SIZE
# define DEFAULT_CLIENT_MAX_BODY_SIZE 1000
#endif

#ifndef DEFAULT_KEEP_ALIVE_TIMEOUT
# define DEFAULT_KEEP_ALIVE_TIMEOUT 65
#endif

#ifndef DEFAULT_TYPE
# define DEFAULT_TYPE "application/octet-stream"
#endif

namespace webserv
{
namespace config
{

class ServerConfig;

class HTTPConfig
{
	public:
		static HTTPConfig					&getInstance();

		int									getClientMaxBodySize() const;
		void								setClientMaxBodySize(const int clientMaxBodySize);

		bool								getCreateFullPutPath() const;
		void								setCreateFullPutPath(const bool createFullPutPath);
		std::string							getDavPutPath() const;
		void								setDavPutPath(const std::string &davPutPath);
		std::string							getDavAccess() const;
		void								setDavAccess(const std::string &davAccess);
		std::vector<config::e_Method>		getDavMethods() const;
		void								setDavMethods(const	 std::vector<config::e_Method> &davMethods);

		std::string							getDefaultType() const;
		void								setDefaultType(const std::string &defaultType);
		std::vector<ErrorPage>				getErrorPage() const;
		void								setErrorPage(const std::vector<ErrorPage> &errorPage);
		int									getKeepAliveTimeout() const;
		void								setKeepAliveTimeout(const int keepAliveTimeout);
		std::string							getRoot() const;
		void								setRoot(const std::string &root);
		std::vector<ServerConfig>			getServerConfigs() const;
		void								setServerConfigs(const std::vector<ServerConfig> &serverConfigs);
		std::map<std::string, std::string>	getTypes() const;
		void								setTypes(const std::map<std::string, std::string> &types);
		bool								getEnableCGI() const;
		void								setEnableCGI(const bool enableCGI);
		std::map<std::string, std::string>	getCgiExtensions() const;
		void								setCgiExtensions(const std::map<std::string, std::string> &cgiExtensions);

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
