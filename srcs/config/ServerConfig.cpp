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
	_logger->setLevel(log42::logRecord::DEBUG);
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
 * @param clientMaxBodySize [TODO:parameter]
 */
void ServerConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
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
 * @param davPutPath [TODO:parameter]
 */
void ServerConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
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
 * @param davAccess [TODO:parameter]
 */
void ServerConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
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
 * @param davMethods [TODO:parameter]
 */
void ServerConfig::setDavMethods(const t_DavMethods &davMethods)
{
	_davMethods = davMethods;
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
 * @param defaultType [TODO:parameter]
 */
void ServerConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
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
 * @param errorPage [TODO:parameter]
 */
void ServerConfig::setErrorPage(const t_ErrorPages &errorPage)
{
	_errorPage = errorPage;
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
 * @param keepAliveTimeout [TODO:parameter]
 */
void ServerConfig::setKeepAliveTimeout(const t_keepAliveTimeout &keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
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
 * @param listen [TODO:parameter]
 */
void ServerConfig::setListen(const t_Listen &listen)
{
	_listen = listen;
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
 * @param locationConfigs [TODO:parameter]
 */
void ServerConfig::setLocationConfigs(const t_LocationConfigs &locationConfigs)
{
	_locationConfigs = locationConfigs;
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
 * @param root [TODO:parameter]
 */
void ServerConfig::setRoot(const std::string &root)
{
	_root = root;
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
 * @param serverName [TODO:parameter]
 */
void ServerConfig::setServerName(const t_Servernames &serverName)
{
	_serverName = serverName;
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
 * @param types [TODO:parameter]
 */
void ServerConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
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
 * @param enableCGI [TODO:parameter]
 */
void ServerConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
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

/**
 * @brief [TODO:description]
 *
 * @param cgiExtensions [TODO:parameter]
 */
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
