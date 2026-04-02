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

		bool						getCreateFullPutPath() const;
		const std::string			&getDavPutPath() const;
		const t_Perms				&getDavAccess() const;
		const t_DavMethods			&getDavMethods() const;

		const std::string			&getDefaultType() const;
		const t_ErrorPages			&getErrorPage() const;
		const t_keepAliveTimeout	&getKeepAliveTimeout() const;
		const t_Listen				&getListen() const;
		const t_LocationConfigs		&getLocationConfigs() const;
		const std::string			&getRoot() const;
		const t_Servernames			&getServerNames() const;
		const t_MimeTypes			&getTypes() const;
		bool						getEnableCGI() const;
		const t_CgiExtensions		&getCgiExtensions() const;
		const LocationConfig		&findLocationConfig(const std::string &requestTarget);

		void				setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize);
		void				setCreateFullPutPath(const bool createFullPutPath);
		void				setDavPutPath(const std::string &davPutPath);
		void				setDavAccess(const t_Perms &davAccess);
		void				setDavMethods(const t_DavMethods &davMethods);
		void				setDefaultType(const std::string &defaultType);
		void				setErrorPage(const t_ErrorPages &errorPage);
		void				setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout);
		void				setListen(const t_Listen &listen);
		void				setLocationConfigs(const t_LocationConfigs &locationConfigs);
		void				setRoot(const std::string &root);
		void				setServerName(const t_Servernames &serverName);
		void				setTypes(const t_MimeTypes &types);
		void				setEnableCGI(const bool enableCGI);
		void				setCgiExtensions(const t_CgiExtensions &cgiExtensions);

	private:
		t_Logger					_logger;
		t_clientMaxBodySize			_clientMaxBodySize;
		bool						_createFullPutPath;
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
