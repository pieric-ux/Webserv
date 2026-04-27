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

		static t_Logger				getLogger();

		const t_clientMaxBodySize	&getClientMaxBodySize() const;
		void						setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize);

		const std::string			&getDavPutPath() const;
		void						setDavPutPath(const std::string &davPutPath);
		const t_Perms				&getDavAccess() const;
		void						setDavAccess(const t_Perms &davAccess);
		const t_DavMethods			&getDavMethods() const;
		void						setDavMethods(const t_DavMethods &davMethods);

		const std::string			&getDefaultType() const;
		void						setDefaultType(const std::string &defaultType);
		const t_ErrorPages			&getErrorPage() const;
		void						setErrorPage(const t_ErrorPages &errorPage);
		const t_keepAliveTimeout	&getKeepAliveTimeout() const;
		void						setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout);
		const t_Listen				&getListen() const;
		void						setListen(const t_Listen &listen);
		const t_LocationConfigs		&getLocationConfigs() const;
		void						setLocationConfigs(const t_LocationConfigs &locationConfigs);
		const std::string			&getRoot() const;
		void						setRoot(const std::string &root);
		const t_Servernames			&getServerNames() const;
		void						setServerName(const t_Servernames &serverName);
		const t_MimeTypes			&getTypes() const;
		void						setTypes(const t_MimeTypes &types);
		bool						isEnableCGI() const;
		void						setEnableCGI(const bool enableCGI);
		const t_CgiExtensions		&getCgiExtensions() const;
		void						setCgiExtensions(const t_CgiExtensions &cgiExtensions);
		const LocationConfig		&findLocationConfig(const std::string &requestTarget);


	private:
		t_Logger					_logger;
		t_clientMaxBodySize			_clientMaxBodySize;
		std::string					_davPutPath;
		t_Perms						_davAccess;
		t_DavMethods				_davMethods;
		std::string					_defaultType;
		t_ErrorPages				_errorPage;
		t_keepAliveTimeout			_keepAliveTimeout;
		t_Listen					_listen;
		t_LocationConfigs			_locationConfigs;
		std::string					_root;
		t_Servernames				_serverName;
		t_MimeTypes					_types;
		bool						_enableCGI;
		t_CgiExtensions				_cgiExtensions;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_SERVERCONFIG_HPP
