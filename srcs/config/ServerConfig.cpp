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
	:	_clientMaxBodySize(DEFAULT_CLIENT_MAX_BODY_SIZE),
		_createFullPutPath(false),
		_keepAliveTimeout(DEFAULT_KEEP_ALIVE_TIMEOUT),
		_enableCGI(false)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.serverconfig");
	_logger->setLevel(log42::logRecord::INFO);
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
int ServerConfig::getClientMaxBodySize() const
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
std::string ServerConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string ServerConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::vector<e_Method> ServerConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string ServerConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::vector<ErrorPage> ServerConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int ServerConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::vector<Listen> ServerConfig::getListen() const
{
	return _listen;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::vector<LocationConfig> ServerConfig::getLocationConfigs() const
{
	return _locationConfigs;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string ServerConfig::getRoot() const
{
	return _root;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::vector<std::string> ServerConfig::getServerName() const
{
	return _serverName;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::map<std::string, std::string> ServerConfig::getTypes() const
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
std::map<std::string, std::string> ServerConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

/**
 * @brief [TODO:description]
 *
 * @param requestTarget [TODO:parameter]
 * @return [TODO:return]
 */
LocationConfig ServerConfig::findLocationConfig(const std::string &requestTarget)
{
	(void)requestTarget;
	return LocationConfig();
}

} // !config
} // !webserv
