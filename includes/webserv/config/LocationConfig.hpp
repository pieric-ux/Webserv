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

		int							getClientMaxBodySize() const;
		bool						getCreateFullPutPath() const;
		const std::string			&getDavPutPath() const;
		const std::string			&getDavAccess() const;
		const t_DavMethods			&getDavMethods() const;

		const std::string			&getDefaultType() const;
		const t_ErrorPages			&getErrorPage() const;
		const t_Index				&getIndex() const;
		int							getKeepAliveTimeout() const;
		const t_AllowedMethods		&getAllowedMethods() const;
		const std::string			&getRoot() const;
		const t_MimeTypes			&getTypes() const;
		const std::string			&getUri() const;
		const e_Modifier			&getModifier() const;
		void						setModifier(const e_Modifier modifier);
		const Return				&getRedirect() const;
		bool						getEnableCGI() const;
		const t_CgiExtensions		&getCgiExtensions() const;

	private:
		t_Logger					_logger;
		bool						_autoindex;
		int							_clientMaxBodySize;
		bool						_createFullPutPath;
		std::string					_davPutPath;
		std::string					_davAccess;
		t_DavMethods				_davMethods;
		std::string					_defaultType;
		t_ErrorPages				_errorPage;
		t_Index						_index;
		int							_keepAliveTimeout;
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
