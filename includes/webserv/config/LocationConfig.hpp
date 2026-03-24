// TODO: don't forget header

#ifndef WEBSERV_CONFIG_LOCATIONCONFIG_HPP
#define WEBSERV_CONFIG_LOCATIONCONFIG_HPP

/**
 * @file LocationConfig.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <map>
#include <string>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/Return.hpp>

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

		bool								getAutoindex() const;

		int									getClientMaxBodySize() const;
		bool								getCreateFullPutPath() const;
		std::string							getDavPutPath() const;
		std::string							getDavAccess() const;
		std::vector<config::e_Method>		getDavMethods() const;

		std::string							getDefaultType() const;
		std::vector<ErrorPage>				getErrorPage() const;
		std::vector<std::string>			getIndex() const;
		int									getKeepAliveTimeout() const;
		std::vector<config::e_Method>		getAllowedMethods() const;
		std::string							getRoot() const;
		std::map<std::string, std::string>	getTypes() const;
		std::string							getUri() const;
		e_Modifier							getModifier() const;
		void								setModifier(const e_Modifier modifier);
		Return								getRedirect() const;
		bool								getEnableCGI() const;
		std::map<std::string, std::string>	getCgiExtensions() const;

	private:
		bool								_autoindex;
		int									_clientMaxBodySize;
		bool								_createFullPutPath;
		std::string							_davPutPath;
		std::string							_davAccess;
		std::vector<config::e_Method>		_davMethods;
		std::string							_defaultType;
		std::vector<ErrorPage>				_errorPage;
		std::vector<std::string>			_index;
		int									_keepAliveTimeout;
		std::vector<int>					_allowedMethods;
		std::string							_root;
		std::map<std::string, std::string>	_types;
		std::string							_uri;
		e_Modifier							_modifier;
		Return								_redirect;
		bool								_enableCGI;
		std::map<std::string, std::string>	_cgiExtensions;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_LOCATIONCONFIG_HPP
