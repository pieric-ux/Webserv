// TODO: don't forget header

/**
 * @file ServerConfig.cpp
 * @brief [TODO:description]
 */

#include <webserv/config/ServerConfig.hpp>

namespace webserv 
{
namespace config
{

/**
 * @brief [TODO:description]
 */
ServerConfig::ServerConfig() 
	:	_clientMaxBodySize(DefaultConfig::clientMaxBodySize),
		_createFullPutPath(DefaultConfig::createFullPutPath),
		_davPutPath(DefaultConfig::davPutPath),
		_davAccess(DefaultConfig::davAccess),
		_davMethods(DefaultConfig::davMethods),
		_defaultType(DefaultConfig::defaultType),
		_errorPage(DefaultConfig::errorPage),
		_keepAliveTimeout(DefaultConfig::keepAliveTimeout),
		_listen(DefaultConfig::listen),
		_locationConfigs(t_LocationConfigs()),
		_root(DefaultConfig::root),
		_serverName(DefaultConfig::servernames),
		_types(DefaultConfig::types),
		_enableCGI(DefaultConfig::enableCGI),
		_cgiExtensions(DefaultConfig::cgiExtensions)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.serverconfig");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "ServerConfig created with default values");
}

/**
 * @brief [TODO:description]
 */
ServerConfig::~ServerConfig() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
ServerConfig::ServerConfig(const ServerConfig &rhs)
	:	_logger(rhs._logger),
		_clientMaxBodySize(rhs._clientMaxBodySize),
		_createFullPutPath(rhs._createFullPutPath),
		_davPutPath(rhs._davPutPath),
		_davAccess(rhs._davAccess),
		_davMethods(rhs._davMethods),
		_defaultType(rhs._defaultType),
		_errorPage(rhs._errorPage),
		_keepAliveTimeout(rhs._keepAliveTimeout),
		_listen(rhs._listen),
		_locationConfigs(rhs._locationConfigs),
		_root(rhs._root),
		_serverName(rhs._serverName),
		_types(rhs._types),
		_enableCGI(rhs._enableCGI),
		_cgiExtensions(rhs._cgiExtensions)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
ServerConfig &ServerConfig::operator=(const ServerConfig &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_createFullPutPath = rhs._createFullPutPath;
		_davPutPath = rhs._davPutPath;
		_davAccess = rhs._davAccess;
		_davMethods = rhs._davMethods;
		_defaultType = rhs._defaultType;
		_errorPage = rhs._errorPage;
		_keepAliveTimeout = rhs._keepAliveTimeout;
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
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	ServerConfig::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.config.serverconfig");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_clientMaxBodySize	&ServerConfig::getClientMaxBodySize() const
{
	return _clientMaxBodySize;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool ServerConfig::getCreateFullPutPath() const
{
	return _createFullPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &ServerConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Perms				&ServerConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_DavMethods	&ServerConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &ServerConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_ErrorPages	&ServerConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_keepAliveTimeout	&ServerConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Listen &ServerConfig::getListen() const
{
	return _listen;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_LocationConfigs	&ServerConfig::getLocationConfigs() const
{
	return _locationConfigs;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &ServerConfig::getRoot() const
{
	return _root;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Servernames	&ServerConfig::getServerNames() const
{
	return _serverName;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_MimeTypes	&ServerConfig::getTypes() const
{
	return _types;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool ServerConfig::getEnableCGI() const
{
	return _enableCGI;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_CgiExtensions	&ServerConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

void ServerConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

void ServerConfig::setCreateFullPutPath(const bool createFullPutPath)
{
	_createFullPutPath = createFullPutPath;
}

void ServerConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
}

void ServerConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
}

void ServerConfig::setDavMethods(const t_DavMethods &davMethods)
{
	_davMethods = davMethods;
}

void ServerConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
}

void ServerConfig::setErrorPage(const t_ErrorPages &errorPage)
{
	_errorPage = errorPage;
}

void ServerConfig::setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
}

void ServerConfig::setListen(const t_Listen &listen)
{
	_listen = listen;
}

void ServerConfig::setLocationConfigs(const t_LocationConfigs &locationConfigs)
{
	_locationConfigs = locationConfigs;
}

void ServerConfig::setRoot(const std::string &root)
{
	_root = root;
}

void ServerConfig::setServerName(const t_Servernames &serverName)
{
	_serverName = serverName;
}

void ServerConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
}

void ServerConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
}

void ServerConfig::setCgiExtensions(const t_CgiExtensions &cgiExtensions)
{
	_cgiExtensions = cgiExtensions;
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 * @return [TODO:return]
 */
const LocationConfig &ServerConfig::findLocationConfig(const std::string &requestTarget)
{
	(void)requestTarget;
	return _locationConfigs.front();
}

} // !config
} // !webserv
