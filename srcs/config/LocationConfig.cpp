/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file LocationConfig.cpp
 * @brief Implements the LocationConfig class, storing the per-location (nginx-style) request-handling directives and providing their accessors and mutators.
 */

#include <webserv/config/LocationConfig.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief Constructs a LocationConfig with every directive initialized from DefaultConfig and the modifier set to PREFIX, then sets up the logger.
 */
LocationConfig::LocationConfig()
	:	_autoindex(DefaultConfig::autoindex),
		_clientMaxBodySize(DefaultConfig::clientMaxBodySize),
		_davPutPath(DefaultConfig::davPutPath),
		_davAccess(DefaultConfig::davAccess),
		_davMethods(DefaultConfig::davMethods),
		_defaultType(DefaultConfig::defaultType),
		_errorPage(DefaultConfig::errorPage),
		_index(DefaultConfig::index),
		_keepAliveTimeout(DefaultConfig::keepAliveTimeout),
		_sessionTTL(DefaultConfig::sessionTTL),
		_allowedMethods(DefaultConfig::allowedMethods),
		_root(DefaultConfig::root),
		_types(DefaultConfig::types),
		_modifier(PREFIX),
		_enableCGI(DefaultConfig::enableCGI),
		_cgiExtensions(DefaultConfig::cgiExtensions)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.locationconfig");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "LocationConfig instance created with default values");
}

/**
 * @brief Destroys the LocationConfig instance.
 */
LocationConfig::~LocationConfig() {}

/**
 * @brief Copy-constructs a LocationConfig by copying every directive member from another instance.
 *
 * @param rhs The LocationConfig to copy from.
 */
LocationConfig::LocationConfig(const LocationConfig &rhs)
	: _autoindex(rhs._autoindex),
	  _clientMaxBodySize(rhs._clientMaxBodySize),
	  _davPutPath(rhs._davPutPath),
	  _davAccess(rhs._davAccess),
	  _davMethods(rhs._davMethods),
	  _defaultType(rhs._defaultType),
	  _errorPage(rhs._errorPage),
	  _index(rhs._index),
	  _keepAliveTimeout(rhs._keepAliveTimeout),
  _sessionTTL(rhs._sessionTTL),
	  _allowedMethods(rhs._allowedMethods),
	  _root(rhs._root),
	  _types(rhs._types),
	  _uri(rhs._uri),
	  _modifier(rhs._modifier),
	  _redirect(rhs._redirect),
	  _enableCGI(rhs._enableCGI),
	  _cgiExtensions(rhs._cgiExtensions)
{}

/**
 * @brief Copy-assigns every directive member from another LocationConfig, guarding against self-assignment.
 *
 * @param rhs The LocationConfig to copy from.
 * @return Reference to this instance after assignment.
 */
LocationConfig &LocationConfig::operator=(const LocationConfig &rhs)
{
	if (this != &rhs)
	{
		_autoindex = rhs._autoindex;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_davPutPath = rhs._davPutPath;
		_davAccess = rhs._davAccess;
		_davMethods = rhs._davMethods;
		_defaultType = rhs._defaultType;
		_errorPage = rhs._errorPage;
		_index = rhs._index;
		_keepAliveTimeout = rhs._keepAliveTimeout;
		_sessionTTL = rhs._sessionTTL;
		_allowedMethods = rhs._allowedMethods;
		_root = rhs._root;
		_types = rhs._types;
		_uri = rhs._uri;
		_modifier = rhs._modifier;
		_redirect = rhs._redirect;
		_enableCGI = rhs._enableCGI;
		_cgiExtensions = rhs._cgiExtensions;
	}
	return (*this);
}

/**
 * @brief Returns the shared logger for the LocationConfig class.
 *
 * @return The "webserv.config.locationconfig" logger instance.
 */
t_Logger	LocationConfig::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.config.locationconfig");
}

/**
 * @brief Reports whether automatic directory listing is enabled for this location.
 *
 * @return true if autoindex is enabled, false otherwise.
 */
bool LocationConfig::getAutoindex() const
{
	return _autoindex;
}

/**
 * @brief Enables or disables automatic directory listing for this location.
 *
 * @param autoindex true to enable autoindex, false to disable it.
 */
void LocationConfig::setAutoindex(const bool autoindex)
{
	_autoindex = autoindex;
}

/**
 * @brief Returns the maximum allowed client request body size for this location.
 *
 * @return Reference to the maximum body size in bytes.
 */
const t_clientMaxBodySize	&LocationConfig::getClientMaxBodySize() const
{
	return _clientMaxBodySize;
}

/**
 * @brief Sets the maximum allowed client request body size for this location.
 *
 * @param clientMaxBodySize The maximum body size in bytes to allow.
 */
void LocationConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

/**
 * @brief Returns the filesystem path used as the destination for WebDAV PUT uploads.
 *
 * @return Reference to the DAV PUT target path.
 */
const std::string &LocationConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief Sets the filesystem path used as the destination for WebDAV PUT uploads.
 *
 * @param davPutPath The DAV PUT target path to store.
 */
void LocationConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
}

/**
 * @brief Returns the access permission bits applied to files created via WebDAV.
 *
 * @return Reference to the DAV access permission mask.
 */
const t_Perms	&LocationConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief Sets the access permission bits applied to files created via WebDAV.
 *
 * @param davAccess The DAV access permission mask to store.
 */
void LocationConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
}

/**
 * @brief Returns the set of HTTP methods enabled for WebDAV handling on this location.
 *
 * @return Reference to the set of enabled DAV methods.
 */
const t_DavMethods	&LocationConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief Sets the set of HTTP methods enabled for WebDAV handling on this location.
 *
 * @param davMethods The set of DAV methods to enable.
 */
void LocationConfig::setDavMethods(const t_DavMethods &davMethods)
{
	_davMethods = davMethods;
}

/**
 * @brief Returns the default MIME type used when a file's type cannot be resolved.
 *
 * @return Reference to the default content type string.
 */
const std::string &LocationConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief Sets the default MIME type used when a file's type cannot be resolved.
 *
 * @param defaultType The default content type string to store.
 */
void LocationConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
}

/**
 * @brief Returns the collection of custom error-page mappings for this location.
 *
 * @return Reference to the list of configured error pages.
 */
const t_ErrorPages	&LocationConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief Sets the collection of custom error-page mappings for this location.
 *
 * @param errorPage The list of error pages to store.
 */
void LocationConfig::setErrorPage(const t_ErrorPages &errorPage)
{
	_errorPage = errorPage;
}

/**
 * @brief Returns the ordered list of index file names tried when a directory is requested.
 *
 * @return Reference to the list of index file names.
 */
const t_Index	&LocationConfig::getIndex() const
{
	return _index;
}

/**
 * @brief Sets the ordered list of index file names tried when a directory is requested.
 *
 * @param index The list of index file names to store.
 */
void LocationConfig::setIndex(const t_Index &index)
{
	_index = index;
}

/**
 * @brief Returns the keep-alive timeout applied to persistent connections for this location.
 *
 * @return Reference to the keep-alive timeout in seconds.
 */
const t_keepAliveTimeout	&LocationConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief Sets the keep-alive timeout applied to persistent connections for this location.
 *
 * @param keepAliveTimeout The keep-alive timeout in seconds to store.
 */
void LocationConfig::setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
}

/**
 * @brief Returns the time-to-live applied to sessions for this location.
 *
 * @return Reference to the session TTL in seconds.
 */
const t_sessionTTL &LocationConfig::getSessionTTL() const
{
	return _sessionTTL;
}

/**
 * @brief Sets the time-to-live applied to sessions for this location.
 *
 * @param sessionTTL The session TTL in seconds to store.
 */
void LocationConfig::setSessionTTL(const t_sessionTTL &sessionTTL)
{
	_sessionTTL = sessionTTL;
}

/**
 * @brief Returns the set of HTTP methods accepted by this location.
 *
 * @return Reference to the set of allowed methods.
 */
const t_AllowedMethods	&LocationConfig::getAllowedMethods() const
{
	return _allowedMethods;
}

/**
 * @brief Sets the set of HTTP methods accepted by this location.
 *
 * @param allowedMethods The set of allowed methods to store.
 */
void LocationConfig::setAllowedMethods(const t_AllowedMethods &allowedMethods)
{
	_allowedMethods = allowedMethods;
}

/**
 * @brief Returns the filesystem root directory against which request URIs are resolved.
 *
 * @return Reference to the root path string.
 */
const std::string &LocationConfig::getRoot() const
{
	return _root;
}

/**
 * @brief Sets the filesystem root directory against which request URIs are resolved.
 *
 * @param root The root path string to store.
 */
void LocationConfig::setRoot(const std::string &root)
{
	_root = root;
}

/**
 * @brief Returns the map associating file extensions with their MIME types for this location.
 *
 * @return Reference to the extension-to-MIME-type map.
 */
const t_MimeTypes	&LocationConfig::getTypes() const
{
	return _types;
}

/**
 * @brief Sets the map associating file extensions with their MIME types for this location.
 *
 * @param types The extension-to-MIME-type map to store.
 */
void LocationConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
}

/**
 * @brief Returns the URI pattern that this location block matches against.
 *
 * @return Reference to the location URI string.
 */
const std::string &LocationConfig::getUri() const
{
	return _uri;
}

/**
 * @brief Sets the URI pattern that this location block matches against.
 *
 * @param uri The location URI string to store.
 */
void LocationConfig::setUri(const std::string &uri)
{
	_uri = uri;
}

/**
 * @brief Returns the URI matching modifier (PREFIX, EXACT or PREFIX_PRIORITY) for this location.
 *
 * @return Reference to the location matching modifier.
 */
const e_Modifier &LocationConfig::getModifier() const
{
	return _modifier;
}

/**
 * @brief Sets the URI matching modifier (PREFIX, EXACT or PREFIX_PRIORITY) for this location.
 *
 * @param modifier The location matching modifier to store.
 */
void LocationConfig::setModifier(const e_Modifier modifier)
{
	_modifier = modifier;
}

/**
 * @brief Returns the redirect (return) directive configured for this location.
 *
 * @return Reference to the redirect descriptor.
 */
const Return &LocationConfig::getRedirect() const
{
	return _redirect;
}

/**
 * @brief Sets the redirect (return) directive configured for this location.
 *
 * @param redirect The redirect descriptor to store.
 */
void LocationConfig::setRedirect(const Return &redirect)
{
	_redirect = redirect;
}

/**
 * @brief Reports whether CGI execution is enabled for this location.
 *
 * @return true if CGI handling is enabled, false otherwise.
 */
bool LocationConfig::isEnableCGI() const
{
	return _enableCGI;
}

/**
 * @brief Enables or disables CGI execution for this location.
 *
 * @param enableCGI true to enable CGI handling, false to disable it.
 */
void LocationConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
}

/**
 * @brief Returns the map associating file extensions with their CGI interpreter paths.
 *
 * @return Reference to the extension-to-CGI-interpreter map.
 */
const t_CgiExtensions	&LocationConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

/**
 * @brief Sets the map associating file extensions with their CGI interpreter paths.
 *
 * @param cgiExtensions The extension-to-CGI-interpreter map to store.
 */
void LocationConfig::setCgiExtensions(const t_CgiExtensions &cgiExtensions)
{
	_cgiExtensions = cgiExtensions;
}

} // !config
} // !webserv
