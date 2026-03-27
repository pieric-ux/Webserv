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
		static HTTPConfig	&getInstance();

		t_Logger			getLogger() const;

		int					getClientMaxBodySize() const;
		void				setClientMaxBodySize(const int clientMaxBodySize);

		bool				getCreateFullPutPath() const;
		void				setCreateFullPutPath(const bool createFullPutPath);
		std::string			getDavPutPath() const;
		void				setDavPutPath(const std::string &davPutPath);
		std::string			getDavAccess() const;
		void				setDavAccess(const std::string &davAccess);
		t_DavMethods		getDavMethods() const;
		void				setDavMethods(const	 t_DavMethods &davMethods);

		std::string			getDefaultType() const;
		void				setDefaultType(const std::string &defaultType);
		t_ErrorPages		getErrorPage() const;
		void				setErrorPage(const t_ErrorPages &errorPage);
		int					getKeepAliveTimeout() const;
		void				setKeepAliveTimeout(const int keepAliveTimeout);
		std::string			getRoot() const;
		void				setRoot(const std::string &root);
		t_ServerConfigs		getServerConfigs() const;
		void				setServerConfigs(const t_ServerConfigs &serverConfigs);
		t_MimeTypes			getTypes() const;
		void				setTypes(const t_MimeTypes &types);
		bool				getEnableCGI() const;
		void				setEnableCGI(const bool enableCGI);
		t_CgiExtensions		getCgiExtensions() const;
		void				setCgiExtensions(const t_CgiExtensions &cgiExtensions);

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
		std::string			_root;
		t_ServerConfigs		_serverConfigs;
		t_MimeTypes			_types;
		bool				_enableCGI;
		t_CgiExtensions		_cgiExtensions;

		HTTPConfig();
		~HTTPConfig();
		HTTPConfig(const HTTPConfig &rhs);
		HTTPConfig &operator=(const HTTPConfig &rhs);
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_HTTPCONFIG_HPP
