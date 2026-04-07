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

		const t_clientMaxBodySize	&getClientMaxBodySize() const;
		bool						getCreateFullPutPath() const;
		const std::string			&getDavPutPath() const;
		const t_Perms				&getDavAccess() const;
		const t_DavMethods			&getDavMethods() const;

		const std::string			&getDefaultType() const;
		const t_ErrorPages			&getErrorPage() const;
		const t_Index				&getIndex() const;
		const t_keepAliveTimeout	&getKeepAliveTimeout() const;
		const t_AllowedMethods		&getAllowedMethods() const;
		const std::string			&getRoot() const;
		const t_MimeTypes			&getTypes() const;
		const std::string			&getUri() const;
		const e_Modifier			&getModifier() const;
		void						setModifier(const e_Modifier modifier);
		const Return				&getRedirect() const;
		bool						getEnableCGI() const;
		const t_CgiExtensions		&getCgiExtensions() const;

		void				setAutoindex(const bool autoindex);
		void				setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize);
		void				setCreateFullPutPath(const bool createFullPutPath);
		void				setDavPutPath(const std::string &davPutPath);
		void				setDavAccess(const t_Perms &davAccess);
		void				setDavMethods(const t_DavMethods &davMethods);
		void				setDefaultType(const std::string &defaultType);
		void				setErrorPage(const t_ErrorPages &errorPage);
		void				setIndex(const t_Index &index);
		void				setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout);
		void				setAllowedMethods(const t_AllowedMethods &allowedMethods);
		void				setRoot(const std::string &root);
		void				setTypes(const t_MimeTypes &types);
		void				setUri(const std::string &uri);
		void				setRedirect(const Return &redirect);
		void				setEnableCGI(const bool enableCGI);
		void				setCgiExtensions(const t_CgiExtensions &cgiExtensions);

	private:
		t_Logger					_logger;
		bool						_autoindex;
		t_clientMaxBodySize			_clientMaxBodySize;
		bool						_createFullPutPath;
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
