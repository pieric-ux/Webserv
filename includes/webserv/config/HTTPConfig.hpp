// TODO: don't forget header

#ifndef WEBSERV_CONFIG_HTTPCONFIG_HPP
#define WEBSERV_CONFIG_HTTPCONFIG_HPP

/**
 * @file HTTPConfig.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/types.hpp>
#include <webserv/config/DefaultConfig.hpp>

namespace webserv
{
namespace config
{

class ServerConfig;

class HTTPConfig
{
	public:
		static HTTPConfig			&getInstance();

		t_Logger					getLogger() const;

		std::string					getIOMultiplexer() const;
		void						setIOMultiplexer(const std::string &ioMultiplexer);

		const t_clientMaxBodySize	&getClientMaxBodySize() const;
		void						setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize);

		bool						getCreateFullPutPath() const;
		void						setCreateFullPutPath(const bool createFullPutPath);
		const std::string			&getDavPutPath() const;
		void						setDavPutPath(const std::string &davPutPath);
		const t_Perms				&getDavAccess() const;
		void						setDavAccess(const t_Perms &davAccess);
		const t_DavMethods			&getDavMethods() const;
		void						setDavMethods(const	 t_DavMethods &davMethods);

		const std::string			&getDefaultType() const;
		void						setDefaultType(const std::string &defaultType);
		const t_ErrorPages			&getErrorPage() const;
		void						setErrorPage(const t_ErrorPages &errorPage);
		t_keepAliveTimeout			getKeepAliveTimeout() const;
		void						setKeepAliveTimeout(const t_keepAliveTimeout keepAliveTimeout);
		const std::string			&getRoot() const;
		void						setRoot(const std::string &root);
		const t_ServerConfigs		&getServerConfigs() const;
		void						setServerConfigs(const t_ServerConfigs &serverConfigs);
		const t_MimeTypes			&getTypes() const;
		void						setTypes(const t_MimeTypes &types);
		bool						getEnableCGI() const;
		void						setEnableCGI(const bool enableCGI);
		const t_CgiExtensions		&getCgiExtensions() const;
		void						setCgiExtensions(const t_CgiExtensions &cgiExtensions);

	private:
		t_Logger					_logger;
		std::string 				_ioMultiplexer;
		t_clientMaxBodySize			_clientMaxBodySize;
		bool						_createFullPutPath;
		std::string					_davPutPath;
		t_Perms						_davAccess;
		t_DavMethods				_davMethods;
		std::string					_defaultType;
		t_ErrorPages				_errorPage;
		t_keepAliveTimeout			_keepAliveTimeout;
		std::string					_root;
		t_ServerConfigs				_serverConfigs;
		t_MimeTypes					_types;
		bool						_enableCGI;
		t_CgiExtensions				_cgiExtensions;

		HTTPConfig();
		~HTTPConfig();
		HTTPConfig(const HTTPConfig &rhs);
		HTTPConfig &operator=(const HTTPConfig &rhs);
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_HTTPCONFIG_HPP
