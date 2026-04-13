// TODO: don't forget header

#ifndef WEBSERV_CONFIG_LOCATIONCONFIG_HPP
#define WEBSERV_CONFIG_LOCATIONCONFIG_HPP

/**
 * @file LocationConfig.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/Return.hpp>
#include <webserv/config/method.hpp>
#include <webserv/config/DefaultConfig.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace config
{

enum e_Modifier
{
	PREFIX,
	EXACT,
	PREFIX_PRIORITY
};

class LocationConfig
{
	public:
		LocationConfig();
		~LocationConfig();

		LocationConfig(const LocationConfig &rhs);
		LocationConfig &operator=(const LocationConfig &rhs);

		static t_Logger				getLogger();

		bool						getAutoindex() const;
		void						setAutoindex(const bool autoindex);

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
		const t_Index				&getIndex() const;
		void						setIndex(const t_Index &index);
		const t_keepAliveTimeout	&getKeepAliveTimeout() const;
		void						setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout);
		const t_AllowedMethods		&getAllowedMethods() const;
		void						setAllowedMethods(const t_AllowedMethods &allowedMethods);
		const std::string			&getRoot() const;
		void						setRoot(const std::string &root);
		const t_MimeTypes			&getTypes() const;
		void						setTypes(const t_MimeTypes &types);
		const std::string			&getUri() const;
		void						setUri(const std::string &uri);
		const e_Modifier			&getModifier() const;
		void						setModifier(const e_Modifier modifier);
		const Return				&getRedirect() const;
		void						setRedirect(const Return &redirect);
		bool						getEnableCGI() const;
		void						setEnableCGI(const bool enableCGI);
		const t_CgiExtensions		&getCgiExtensions() const;
		void						setCgiExtensions(const t_CgiExtensions &cgiExtensions);


	private:
		t_Logger					_logger;
		bool						_autoindex;
		t_clientMaxBodySize			_clientMaxBodySize;
		std::string					_davPutPath;
		t_Perms						_davAccess;
		t_DavMethods				_davMethods;
		std::string					_defaultType;
		t_ErrorPages				_errorPage;
		t_Index						_index;
		t_keepAliveTimeout			_keepAliveTimeout;
		t_AllowedMethods			_allowedMethods;
		std::string					_root;
		t_MimeTypes					_types;
		std::string					_uri;
		e_Modifier					_modifier;
		Return						_redirect;
		bool						_enableCGI;
		t_CgiExtensions				_cgiExtensions;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_LOCATIONCONFIG_HPP
