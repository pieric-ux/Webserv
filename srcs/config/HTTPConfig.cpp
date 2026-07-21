/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file HTTPConfig.cpp
 * @brief Implements the HTTPConfig singleton and its accessors for the global HTTP-level server configuration.
 */

#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/ServerConfig.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief Constructs the configuration with all fields initialized to their DefaultConfig values and sets up its logger.
 */
HTTPConfig::HTTPConfig()
	:	_ioMultiplexer(DefaultConfig::ioMultiplexer),
		_clientMaxBodySize(DefaultConfig::clientMaxBodySize),
		_davPutPath(DefaultConfig::davPutPath),
		_davAccess(DefaultConfig::davAccess),
		_davMethods(DefaultConfig::davMethods),
		_defaultType(DefaultConfig::defaultType),
		_errorPage(DefaultConfig::errorPage),
		_keepAliveTimeout(DefaultConfig::keepAliveTimeout),
		_sessionTTL(DefaultConfig::sessionTTL),
		_root(DefaultConfig::root),
		_serverConfigs(t_ServerConfigs()),
		_types(DefaultConfig::types),
		_enableCGI(DefaultConfig::enableCGI),
		_cgiExtensions(DefaultConfig::cgiExtensions)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.httpconfig");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "HTTPConfig instance created with default values");
}

/**
 * @brief Destroys the configuration instance.
 */
HTTPConfig::~HTTPConfig() {}

/**
 * @brief Accesses the unique HTTPConfig instance, creating it lazily on first call.
 *
 * @return Reference to the singleton HTTPConfig.
 */
HTTPConfig &HTTPConfig::getInstance()
{
	static HTTPConfig instance;
	return instance;
}

/**
 * @brief Returns the logger used by this configuration object.
 *
 * @return Shared pointer to the HTTPConfig logger.
 */
t_Logger	HTTPConfig::getLogger() const
{
	return _logger;
}

/**
 * @brief Returns the name of the configured I/O multiplexer (e.g. poll or select).
 *
 * @return The I/O multiplexer identifier.
 */
std::string HTTPConfig::getIOMultiplexer() const
{
	return _ioMultiplexer;
}

/**
 * @brief Sets the name of the I/O multiplexer to use.
 *
 * @param ioMultiplexer The I/O multiplexer identifier to store.
 */
void HTTPConfig::setIOMultiplexer(const std::string &ioMultiplexer)
{
	_ioMultiplexer = ioMultiplexer;
}

/**
 * @brief Returns the default maximum allowed client request body size in bytes.
 *
 * @return Reference to the client max body size.
 */
const t_clientMaxBodySize	&HTTPConfig::getClientMaxBodySize() const 
{
	return _clientMaxBodySize;
}

/**
 * @brief Sets the default maximum allowed client request body size in bytes.
 *
 * @param clientMaxBodySize The maximum body size to store.
 */
void	HTTPConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

/**
 * @brief Returns the default filesystem path used as the target for WebDAV PUT uploads.
 *
 * @return Reference to the DAV PUT path.
 */
const std::string &HTTPConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief Sets the default filesystem path used as the target for WebDAV PUT uploads.
 *
 * @param davPutPath The DAV PUT path to store.
 */
void	HTTPConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
}

/**
 * @brief Returns the default WebDAV access permission bits.
 *
 * @return Reference to the DAV access permissions.
 */
const t_Perms &HTTPConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief Sets the default WebDAV access permission bits.
 *
 * @param davAccess The DAV access permissions to store.
 */
void	HTTPConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
}

/**
 * @brief Returns the default set of HTTP methods enabled for WebDAV operations.
 *
 * @return Reference to the set of allowed DAV methods.
 */
const t_DavMethods &HTTPConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief Sets the default set of HTTP methods enabled for WebDAV operations.
 *
 * @param davMethods The set of allowed DAV methods to store.
 */
void	HTTPConfig::setDavMethods(const	t_DavMethods &davMethods)
{
	_davMethods = davMethods;
}

/**
 * @brief Returns the default MIME type used when a response content type cannot be determined.
 *
 * @return Reference to the default MIME type.
 */
const std::string &HTTPConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief Sets the default MIME type used when a response content type cannot be determined.
 *
 * @param defaultType The default MIME type to store.
 */
void	HTTPConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
}

/**
 * @brief Returns the default list of error pages mapping status codes to custom error documents.
 *
 * @return Reference to the vector of error page mappings.
 */
const std::vector<ErrorPage> &HTTPConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief Sets the default list of error pages mapping status codes to custom error documents.
 *
 * @param errorPage The vector of error page mappings to store.
 */
void	HTTPConfig::setErrorPage(const std::vector<ErrorPage> &errorPage)
{
	_errorPage = errorPage;
}

/**
 * @brief Returns the default keep-alive timeout (in seconds) for persistent connections.
 *
 * @return The keep-alive timeout value.
 */
t_keepAliveTimeout HTTPConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief Sets the default keep-alive timeout (in seconds) for persistent connections.
 *
 * @param keepAliveTimeout The keep-alive timeout value to store.
 */
void	HTTPConfig::setKeepAliveTimeout(const t_keepAliveTimeout keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
}

/**
 * @brief Returns the default session time-to-live (in seconds).
 *
 * @return The session TTL value.
 */
t_sessionTTL HTTPConfig::getSessionTTL() const
{
	return _sessionTTL;
}

/**
 * @brief Sets the default session time-to-live (in seconds).
 *
 * @param sessionTTL The session TTL value to store.
 */
void HTTPConfig::setSessionTTL(const t_sessionTTL sessionTTL)
{
	_sessionTTL = sessionTTL;
}

/**
 * @brief Returns the default root directory from which files are served.
 *
 * @return Reference to the root directory path.
 */
const std::string &HTTPConfig::getRoot() const
{
	return _root;
}

/**
 * @brief Sets the default root directory from which files are served.
 *
 * @param root The root directory path to store.
 */
void	HTTPConfig::setRoot(const std::string &root)
{
	_root = root;
}

/**
 * @brief Returns the list of parsed server blocks contained in the HTTP configuration.
 *
 * @return Reference to the vector of server configurations.
 */
const std::vector<ServerConfig> &HTTPConfig::getServerConfigs() const
{
	return _serverConfigs;
}

/**
 * @brief Sets the list of server blocks contained in the HTTP configuration.
 *
 * @param serverConfigs The vector of server configurations to store.
 */
void	HTTPConfig::setServerConfigs(const std::vector<ServerConfig> &serverConfigs)
{
	_serverConfigs = serverConfigs;
}

/**
 * @brief Returns the MIME type table mapping file extensions to content types.
 *
 * @return Reference to the MIME types map.
 */
const t_MimeTypes &HTTPConfig::getTypes() const
{
	return _types;
}

/**
 * @brief Sets the MIME type table mapping file extensions to content types.
 *
 * @param types The MIME types map to store.
 */
void	HTTPConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
}

/**
 * @brief Indicates whether CGI execution is enabled by default.
 *
 * @return true if CGI is enabled, false otherwise.
 */
bool HTTPConfig::isEnableCGI() const
{
	return _enableCGI;
}

/**
 * @brief Enables or disables CGI execution by default.
 *
 * @param enableCGI true to enable CGI, false to disable it.
 */
void	HTTPConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
}

/**
 * @brief Returns the table mapping CGI file extensions to their interpreter executables.
 *
 * @return Reference to the CGI extensions map.
 */
const t_CgiExtensions &HTTPConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

/**
 * @brief Sets the table mapping CGI file extensions to their interpreter executables.
 *
 * @param cgiExtensions The CGI extensions map to store.
 */
void	HTTPConfig::setCgiExtensions(const t_CgiExtensions &cgiExtensions)
{
	_cgiExtensions = cgiExtensions;
}

} // !config
} // !webserv
