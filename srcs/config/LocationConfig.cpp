// TODO: don't forget header

/**
 * @file LocationConfig.cpp
 * @brief [TODO:description]
 */

#include <webserv/config/LocationConfig.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief [TODO:description]
 */
LocationConfig::LocationConfig()
	:	_autoindex(DefaultConfig::autoindex),
		_clientMaxBodySize(DefaultConfig::clientMaxBodySize),
		_createFullPutPath(DefaultConfig::createFullPutPath),
		_davPutPath(DefaultConfig::davPutPath),
		_davAccess(DefaultConfig::davAccess),
		_davMethods(DefaultConfig::davMethods),
		_defaultType(DefaultConfig::defaultType),
		_errorPage(DefaultConfig::errorPage),
		_index(DefaultConfig::index),
		_keepAliveTimeout(DefaultConfig::keepAliveTimeout),
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
 * @brief [TODO:description]
 */
LocationConfig::~LocationConfig() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
LocationConfig::LocationConfig(const LocationConfig &rhs)
	: _autoindex(rhs._autoindex),
	  _clientMaxBodySize(rhs._clientMaxBodySize),
	  _createFullPutPath(rhs._createFullPutPath),
	  _davPutPath(rhs._davPutPath),
	  _davAccess(rhs._davAccess),
	  _davMethods(rhs._davMethods),
	  _defaultType(rhs._defaultType),
	  _errorPage(rhs._errorPage),
	  _index(rhs._index),
	  _keepAliveTimeout(rhs._keepAliveTimeout),
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
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
LocationConfig &LocationConfig::operator=(const LocationConfig &rhs)
{
	if (this != &rhs)
	{
		_autoindex = rhs._autoindex;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_createFullPutPath = rhs._createFullPutPath;
		_davPutPath = rhs._davPutPath;
		_davAccess = rhs._davAccess;
		_davMethods = rhs._davMethods;
		_defaultType = rhs._defaultType;
		_errorPage = rhs._errorPage;
		_index = rhs._index;
		_keepAliveTimeout = rhs._keepAliveTimeout;
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
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	LocationConfig::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.config.locationconfig");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool LocationConfig::getAutoindex() const
{
	return _autoindex;
}
/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_clientMaxBodySize	&LocationConfig::getClientMaxBodySize() const
{
	return _clientMaxBodySize;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool LocationConfig::getCreateFullPutPath() const
{
	return _createFullPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &LocationConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Perms				&LocationConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_DavMethods	&LocationConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &LocationConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_ErrorPages	&LocationConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Index	&LocationConfig::getIndex() const
{
	return _index;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_keepAliveTimeout	&LocationConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &LocationConfig::getRoot() const
{
	return _root;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_MimeTypes	&LocationConfig::getTypes() const
{
	return _types;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &LocationConfig::getUri() const
{
	return _uri;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const e_Modifier &LocationConfig::getModifier() const
{
	return _modifier;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_AllowedMethods	&LocationConfig::getAllowedMethods() const
{
	return _allowedMethods;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool LocationConfig::getEnableCGI() const
{
	return _enableCGI;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const Return &LocationConfig::getRedirect() const
{
	return _redirect;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_CgiExtensions	&LocationConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

/**
 * @brief [TODO:description]
 *
 * @param modifier [TODO:parameter]
 */
void LocationConfig::setModifier(const e_Modifier modifier)
{
	_modifier = modifier;
}

void LocationConfig::setAutoindex(const bool autoindex)
{
	_autoindex = autoindex;
}

void LocationConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

void LocationConfig::setCreateFullPutPath(const bool createFullPutPath)
{
	_createFullPutPath = createFullPutPath;
}

void LocationConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
}

void LocationConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
}

void LocationConfig::setDavMethods(const t_DavMethods &davMethods)
{
	_davMethods = davMethods;
}

void LocationConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
}

void LocationConfig::setErrorPage(const t_ErrorPages &errorPage)
{
	_errorPage = errorPage;
}

void LocationConfig::setIndex(const t_Index &index)
{
	_index = index;
}

void LocationConfig::setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
}

void LocationConfig::setAllowedMethods(const t_AllowedMethods &allowedMethods)
{
	_allowedMethods = allowedMethods;
}

void LocationConfig::setRoot(const std::string &root)
{
	_root = root;
}

void LocationConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
}

void LocationConfig::setUri(const std::string &uri)
{
	_uri = uri;
}

void LocationConfig::setRedirect(const Return &redirect)
{
	_redirect = redirect;
}

void LocationConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
}

void LocationConfig::setCgiExtensions(const t_CgiExtensions &cgiExtensions)
{
	_cgiExtensions = cgiExtensions;
}

} // !config
} // !webserv
