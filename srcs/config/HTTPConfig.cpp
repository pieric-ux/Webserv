// TODO: don't forgot header

/**
 * @file HTTPConfig.cpp
 * @brief [TODO:description]
 */

#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/ServerConfig.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief [TODO:description]
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
 * @brief [TODO:description]
 */
HTTPConfig::~HTTPConfig() {}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
HTTPConfig &HTTPConfig::getInstance()
{
	static HTTPConfig instance;
	return instance;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	HTTPConfig::getLogger() const
{
	return _logger;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::string HTTPConfig::getIOMultiplexer() const
{
	return _ioMultiplexer;
}

/**
 * @brief [TODO:description]
 *
 * @param ioMultiplexer [TODO:parameter]
 */
void HTTPConfig::setIOMultiplexer(const std::string &ioMultiplexer)
{
	_ioMultiplexer = ioMultiplexer;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_clientMaxBodySize	&HTTPConfig::getClientMaxBodySize() const 
{
	return _clientMaxBodySize;
}

/**
 * @brief [TODO:description]
 *
 * @param clientMaxBodySize [TODO:parameter]
 */
void	HTTPConfig::setClientMaxBodySize(const t_clientMaxBodySize &clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &HTTPConfig::getDavPutPath() const
{
	return _davPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @param davPutPath [TODO:parameter]
 */
void	HTTPConfig::setDavPutPath(const std::string &davPutPath)
{
	_davPutPath = davPutPath;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Perms &HTTPConfig::getDavAccess() const
{
	return _davAccess;
}

/**
 * @brief [TODO:description]
 *
 * @param davAccess [TODO:parameter]
 */
void	HTTPConfig::setDavAccess(const t_Perms &davAccess)
{
	_davAccess = davAccess;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_DavMethods &HTTPConfig::getDavMethods() const
{
	return _davMethods;
}

/**
 * @brief [TODO:description]
 *
 * @param davMethods [TODO:parameter]
 */
void	HTTPConfig::setDavMethods(const	t_DavMethods &davMethods)
{
	_davMethods = davMethods;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &HTTPConfig::getDefaultType() const
{
	return _defaultType;
}

/**
 * @brief [TODO:description]
 *
 * @param defaultType [TODO:parameter]
 */
void	HTTPConfig::setDefaultType(const std::string &defaultType)
{
	_defaultType = defaultType;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::vector<ErrorPage> &HTTPConfig::getErrorPage() const
{
	return _errorPage;
}

/**
 * @brief [TODO:description]
 *
 * @param errorPage [TODO:parameter]
 */
void	HTTPConfig::setErrorPage(const std::vector<ErrorPage> &errorPage)
{
	_errorPage = errorPage;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_keepAliveTimeout HTTPConfig::getKeepAliveTimeout() const
{
	return _keepAliveTimeout;
}

/**
 * @brief [TODO:description]
 *
 * @param keepAliveTimeout [TODO:parameter]
 */
void	HTTPConfig::setKeepAliveTimeout(const t_keepAliveTimeout keepAliveTimeout)
{
	_keepAliveTimeout = keepAliveTimeout;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::string &HTTPConfig::getRoot() const
{
	return _root;
}

/**
 * @brief [TODO:description]
 *
 * @param root [TODO:parameter]
 */
void	HTTPConfig::setRoot(const std::string &root)
{
	_root = root;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const std::vector<ServerConfig> &HTTPConfig::getServerConfigs() const
{
	return _serverConfigs;
}

/**
 * @brief [TODO:description]
 *
 * @param serverConfigs [TODO:parameter]
 */
void	HTTPConfig::setServerConfigs(const std::vector<ServerConfig> &serverConfigs)
{
	_serverConfigs = serverConfigs;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_MimeTypes &HTTPConfig::getTypes() const
{
	return _types;
}

/**
 * @brief [TODO:description]
 *
 * @param types [TODO:parameter]
 */
void	HTTPConfig::setTypes(const t_MimeTypes &types)
{
	_types = types;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool HTTPConfig::getEnableCGI() const
{
	return _enableCGI;
}

/**
 * @brief [TODO:description]
 *
 * @param enableCGI [TODO:parameter]
 */
void	HTTPConfig::setEnableCGI(const bool enableCGI)
{
	_enableCGI = enableCGI;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_CgiExtensions &HTTPConfig::getCgiExtensions() const
{
	return _cgiExtensions;
}

/**
 * @brief [TODO:description]
 *
 * @param cgiExtensions [TODO:parameter]
 */
void	HTTPConfig::setCgiExtensions(const t_CgiExtensions &cgiExtensions)
{
	_cgiExtensions = cgiExtensions;
}

} // !config
} // !webserv
