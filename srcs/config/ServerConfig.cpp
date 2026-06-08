/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file ServerConfig.cpp
 * @brief Implements ServerConfig: the settings of a single server block
 *        (listen, server names, root, error pages, MIME types, CGI, WebDAV and
 *        its location blocks) with their accessors and request-target matching.
 */

#include <webserv/config/ServerConfig.hpp>

namespace webserv 
{
namespace config
{

/**
 * @brief Constructs a ServerConfig with every member initialized to its
 *        DefaultConfig value and sets up the module logger at DEBUG level.
 */
ServerConfig::ServerConfig()
	:	_clientMaxBodySize(DefaultConfig::clientMaxBodySize),
		_davPutPath(DefaultConfig::davPutPath),
		_davAccess(DefaultConfig::davAccess),
		_davMethods(DefaultConfig::davMethods),
		_defaultType(DefaultConfig::defaultType),
		_errorPage(DefaultConfig::errorPage),
		_keepAliveTimeout(DefaultConfig::keepAliveTimeout),
		_sessionTTL(DefaultConfig::sessionTTL),
		_listen(DefaultConfig::listen),
		_locationConfigs(t_LocationConfigs()),
		_root(DefaultConfig::root),
		_serverName(DefaultConfig::servernames),
		_types(DefaultConfig::types),
		_enableCGI(DefaultConfig::enableCGI),
		_cgiExtensions(DefaultConfig::cgiExtensions)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.serverconfig");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ServerConfig created with default values");
}

/**
 * @brief Destroys the ServerConfig; members release their own resources.
 */
ServerConfig::~ServerConfig() {}

/**
 * @brief Copy-constructs a ServerConfig by duplicating every member of rhs.
 *
 * @param rhs The ServerConfig instance to copy from.
 */
ServerConfig::ServerConfig(const ServerConfig &rhs)
	:	_logger(rhs._logger),
		_clientMaxBodySize(rhs._clientMaxBodySize),
		_davPutPath(rhs._davPutPath),
		_davAccess(rhs._davAccess),
		_davMethods(rhs._davMethods),
		_defaultType(rhs._defaultType),
		_errorPage(rhs._errorPage),
		_keepAliveTimeout(rhs._keepAliveTimeout),
		_sessionTTL(rhs._sessionTTL),
		_listen(rhs._listen),
		_locationConfigs(rhs._locationConfigs),
		_root(rhs._root),
		_serverName(rhs._serverName),
		_types(rhs._types),
		_enableCGI(rhs._enableCGI),
		_cgiExtensions(rhs._cgiExtensions)
{}

/**
 * @brief Copy-assigns every member from rhs, guarding against self-assignment.
 *
 * @param rhs The ServerConfig instance to copy from.
 * @return Reference to this ServerConfig after assignment.
 */
ServerConfig &ServerConfig::operator=(const ServerConfig &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_davPutPath = rhs._davPutPath;
		_davAccess = rhs._davAccess;
		_davMethods = rhs._davMethods;
		_defaultType = rhs._defaultType;
		_errorPage = rhs._errorPage;
		_keepAliveTimeout = rhs._keepAliveTimeout;
		_sessionTTL = rhs._sessionTTL;
		_listen = rhs._listen;
		_locationConfigs = rhs._locationConfigs;
		_root = rhs._root;
		_serverName = rhs._serverName;
		_types = rhs._types;
		_enableCGI = rhs._enableCGI;
		_cgiExtensions = rhs._cgiExtensions;
	}
	return (*this);
}

/**
 * @brief Retrieves the shared logger for the config.serverconfig module.
 *
 * @return Shared pointer to the "webserv.config.serverconfig" logger.
 */
t_Logger	ServerConfig::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.config.serverconfig");
}

/**
 * @brief Returns the maximum allowed request body size for this server.
 *
 * @return Reference to the configured client max body size, in bytes.
 */
const t_clientMaxBodySize	&ServerConfig::getClientMaxBodySize() const
{
	return _clientMaxBodySize;
}

/**
 * @brief Sets the maximum allowed request body size for this server.
 *
 * @param clientMaxBodySize The new client max body size, in bytes.
 */
void ServerConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

/**
 * @brief Returns the filesystem path used for WebDAV PUT uploads.
 *
 * @return Reference to the configured WebDAV PUT path.
 */
const std::string &ServerConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief Sets the filesystem path used for WebDAV PUT uploads.
 *
 * @param davPutPath The new WebDAV PUT path.
 */
void ServerConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
}

/**
 * @brief Returns the WebDAV access permission bitmask for this server.
 *
 * @return Reference to the configured WebDAV access permissions.
 */
const t_Perms				&ServerConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief Sets the WebDAV access permission bitmask for this server.
 *
 * @param davAccess The new WebDAV access permissions.
 */
void ServerConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
}

/**
 * @brief Returns the set of WebDAV methods enabled for this server.
 *
 * @return Reference to the set of allowed WebDAV methods.
 */
const t_DavMethods	&ServerConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief Sets the set of WebDAV methods enabled for this server.
 *
 * @param davMethods The new set of allowed WebDAV methods.
 */
void ServerConfig::setDavMethods(const t_DavMethods &davMethods)
{
	_davMethods = davMethods;
}

/**
 * @brief Returns the default MIME type used when no mapping matches.
 *
 * @return Reference to the configured default content type.
 */
const std::string &ServerConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief Sets the default MIME type used when no mapping matches.
 *
 * @param defaultType The new default content type.
 */
void ServerConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
}

/**
 * @brief Returns the custom error page mappings configured for this server.
 *
 * @return Reference to the collection of configured error pages.
 */
const t_ErrorPages	&ServerConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief Sets the custom error page mappings for this server.
 *
 * @param errorPage The new collection of error pages.
 */
void ServerConfig::setErrorPage(const t_ErrorPages &errorPage)
{
	_errorPage = errorPage;
}

/**
 * @brief Returns the keep-alive timeout for persistent connections.
 *
 * @return Reference to the configured keep-alive timeout, in seconds.
 */
const t_keepAliveTimeout	&ServerConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief Sets the keep-alive timeout for persistent connections.
 *
 * @param keepAliveTimeout The new keep-alive timeout, in seconds.
 */
void ServerConfig::setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
}

/**
 * @brief Returns the time-to-live applied to sessions for this server.
 *
 * @return Reference to the configured session TTL, in seconds.
 */
const t_sessionTTL &ServerConfig::getSessionTTL() const
{
	return _sessionTTL;
}

/**
 * @brief Sets the time-to-live applied to sessions for this server.
 *
 * @param sessionTTL The new session TTL, in seconds.
 */
void ServerConfig::setSessionTTL(const t_sessionTTL &sessionTTL)
{
	_sessionTTL = sessionTTL;
}

/**
 * @brief Returns the address/port endpoints this server listens on.
 *
 * @return Reference to the collection of configured listen directives.
 */
const t_Listen &ServerConfig::getListen() const
{
	return _listen;
}

/**
 * @brief Sets the address/port endpoints this server listens on.
 *
 * @param listen The new collection of listen directives.
 */
void ServerConfig::setListen(const t_Listen &listen)
{
	_listen = listen;
}

/**
 * @brief Returns the location blocks defined within this server block.
 *
 * @return Reference to the collection of location configurations.
 */
const t_LocationConfigs	&ServerConfig::getLocationConfigs() const
{
	return _locationConfigs;
}

/**
 * @brief Sets the location blocks defined within this server block.
 *
 * @param locationConfigs The new collection of location configurations.
 */
void ServerConfig::setLocationConfigs(const t_LocationConfigs &locationConfigs)
{
	_locationConfigs = locationConfigs;
}

/**
 * @brief Returns the document root directory for this server.
 *
 * @return Reference to the configured root path.
 */
const std::string &ServerConfig::getRoot() const
{
	return _root;
}

/**
 * @brief Sets the document root directory for this server.
 *
 * @param root The new root path.
 */
void ServerConfig::setRoot(const std::string &root)
{
	_root = root;
}

/**
 * @brief Returns the server names (virtual hosts) handled by this server.
 *
 * @return Reference to the collection of configured server names.
 */
const t_Servernames	&ServerConfig::getServerNames() const
{
	return _serverName;
}

/**
 * @brief Sets the server names (virtual hosts) handled by this server.
 *
 * @param serverName The new collection of server names.
 */
void ServerConfig::setServerName(const t_Servernames &serverName)
{
	_serverName = serverName;
}

/**
 * @brief Returns the MIME type mappings (extension to content type).
 *
 * @return Reference to the configured MIME type map.
 */
const t_MimeTypes	&ServerConfig::getTypes() const
{
	return _types;
}

/**
 * @brief Sets the MIME type mappings (extension to content type).
 *
 * @param types The new MIME type map.
 */
void ServerConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
}

/**
 * @brief Reports whether CGI execution is enabled for this server.
 *
 * @return True if CGI is enabled, false otherwise.
 */
bool ServerConfig::isEnableCGI() const
{
	return _enableCGI;
}

/**
 * @brief Enables or disables CGI execution for this server.
 *
 * @param enableCGI True to enable CGI, false to disable it.
 */
void ServerConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
}

/**
 * @brief Returns the CGI extension mappings (extension to interpreter).
 *
 * @return Reference to the configured CGI extension map.
 */
const t_CgiExtensions	&ServerConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

/**
 * @brief Sets the CGI extension mappings (extension to interpreter).
 *
 * @param cgiExtensions The new CGI extension map.
 */
void ServerConfig::setCgiExtensions(const t_CgiExtensions &cgiExtensions)
{
	_cgiExtensions = cgiExtensions;
}

/**
 * @brief Selects the location block matching a request target, preferring an
 *        exact match and otherwise the longest matching prefix; falls back to
 *        the first location when none matches.
 *
 * @param path The request target path to resolve against the locations.
 * @return Reference to the best-matching LocationConfig, or the first location
 *         block if no match is found.
 */
const LocationConfig &ServerConfig::findLocationConfig(const std::string &path)
{
	t_LocationConfigs::iterator bestMatch = _locationConfigs.end();
	std::size_t bestLen = 0;

	t_LocationConfigs::iterator it = _locationConfigs.begin();
	for (; it != _locationConfigs.end(); ++it)
	{
		const std::string &uri = it->getUri();
		if (it->getModifier() == EXACT)
		{
			if (path == uri)
			{
				INFO(_logger, "Exact match found for location: " + uri);
				return (*it);
			}
		}
		else if (path.compare(0, uri.size(), uri) == 0 && uri.size() > bestLen)
		{
			bestLen = uri.size();
			bestMatch = it;
		}
	}

	if (bestMatch != _locationConfigs.end())
	{
		std::string matchType = (bestMatch->getModifier() == PREFIX_PRIORITY) ? "Prefix-priority" : "Prefix";
		INFO(_logger, matchType + " match found for location: " + bestMatch->getUri());
		return (*bestMatch);
	}

	INFO(_logger, "No match found for request target path: " + path);
	return _locationConfigs.front();
}

} // !config
} // !webserv
