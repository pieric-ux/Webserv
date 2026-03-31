// TODO: don't forget header

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

#include <webserv/HTTPServer.hpp>

namespace webserv
{

/**
 * @brief [TODO:description]
 */
HTTPServer::HTTPServer() :	_defaultConfigPath(DEFAULT_CONFIG_PATH),
							_parser(),
							_httpConfig(config::HTTPConfig::getInstance()),
							_serverFactory(),
							_servers(),
							_ioMultiplexer(),
							_clientHandler(),
							_headerRegistry(HTTPheaders::HTTPHeadersRegistry::getInstance()),
							_statusCodeRegistry(status::StatusCodeRegistry::getInstance()),
							_typesRegistry(types::TypesRegistry::getInstance())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "HTTPServer instance created");
}

/**
 * @brief [TODO:description]
 */
HTTPServer::~HTTPServer() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
HTTPServer::HTTPServer(const HTTPServer &rhs)
	:	_logger(rhs._logger),
		_defaultConfigPath(rhs._defaultConfigPath),
		_parser(rhs._parser),
		_httpConfig(rhs._httpConfig),
		_serverFactory(rhs._serverFactory),
		_servers(rhs._servers),
		_ioMultiplexer(rhs._ioMultiplexer),
		_clientHandler(rhs._clientHandler),
		_headerRegistry(rhs._headerRegistry),
		_statusCodeRegistry(rhs._statusCodeRegistry),
		_typesRegistry(rhs._typesRegistry)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
HTTPServer &HTTPServer::operator=(const HTTPServer &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_defaultConfigPath = rhs._defaultConfigPath;
		_parser = rhs._parser;
		_serverFactory = rhs._serverFactory;
		_servers = rhs._servers;
		_ioMultiplexer = rhs._ioMultiplexer;
		_clientHandler = rhs._clientHandler;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	HTTPServer::getLogger() const
{
	return _logger;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
HTTPServer &HTTPServer::getInstance()
{
	static HTTPServer instance;
	return instance;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool HTTPServer::running()
{
	return false;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
void	HTTPServer::loadConfig()
{

}

/**
 * @brief [TODO:description]
 */
void HTTPServer::connectClient()
{

}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const config::HTTPConfig &HTTPServer::getHTTPConfig() const
{
	return _httpConfig;
}

/**
 * @brief [TODO:description]
 *
 * @param httpConfig [TODO:parameter]
 */
void HTTPServer::setHTTPconfig(const config::HTTPConfig &httpConfig)
{
	(void)httpConfig;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const ServerFactory &HTTPServer::getServerFactory() const
{
	return _serverFactory;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_Servers &HTTPServer::getServers() const
{
	return _servers;
}

/**
 * @brief [TODO:description]
 *
 * @param server [TODO:parameter]
 */
void HTTPServer::setServers(const t_Servers &servers)
{
	_servers = servers;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_ioMultiplexer	&HTTPServer::getIOMultiplexer() const
{
	return _ioMultiplexer;
}

} // !webserv
