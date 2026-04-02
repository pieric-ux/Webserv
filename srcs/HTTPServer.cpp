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
HTTPServer::HTTPServer() :	_defaultConfigPath(config::DefaultConfig::defaultConfigPath),
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
HTTPServer &HTTPServer::getInstance()
{
	static HTTPServer instance;
	return instance;
}

/**
 * @brief [TODO:description]
 */
void	HTTPServer::setup()
{
	INFO(_logger, "Setting up HTTPServer...");

	try{
		loadConfig();
	} catch (const std::exception &e)
	{
		throw;
	}

	_servers = _serverFactory.createServers();
	if (_servers.empty())
	{
		CRITICAL(_logger, "No servers created. Check configuration.");
		throw std::runtime_error("No servers created. Check configuration.");
	}

	_ioMultiplexer = t_ioMultiplexer(common::core::io::EventFactoryIO::create(_httpConfig.getIOMultiplexer()));
	INFO(_logger, "I/O multiplexer initialized: " + _httpConfig.getIOMultiplexer());

	t_Servers::const_iterator it = _servers.begin();
	for (; it != _servers.end(); ++it)
	{
    	const t_ServerSockets &sockets = it->getSockets();
		t_ServerSockets::const_iterator socketIt = sockets.begin();
		for (; socketIt != sockets.end(); ++socketIt)
		{
			_ioMultiplexer->add(socketIt->second.getFd(), common::core::io::IEventIO::E_IN);
			INFO(_logger, "Registered fd=" + common::core::utils::toString(socketIt->second.getFd()) + " to I/O multiplexer");
		}
	}

	_clientHandler.setIoMultiplexer(_ioMultiplexer);
	INFO(_logger, "Client handler initialized with I/O multiplexer");

	INFO(_logger, "HTTPServer setup complete");
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
 * @param configPath [TODO:parameter]
 */
void HTTPServer::setConfigPath(const std::string &configPath)
{
	_defaultConfigPath = configPath;
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
 * @return [TODO:return]
 */
void	HTTPServer::loadConfig()
{
	std::ifstream configFile(_defaultConfigPath.c_str());
	if (!configFile.is_open())
	{
		CRITICAL(_logger, "Failed to open config file: " + _defaultConfigPath);
		throw std::runtime_error("Failed to open config file: " + _defaultConfigPath);
	}
	t_raw configContent((std::istreambuf_iterator<char>(configFile)),
							std::istreambuf_iterator<char>());
	configFile.close();
	_parser.parseConfig(configContent);
}

/**
 * @brief [TODO:description]
 */
void HTTPServer::connectClient()
{

}

} // !webserv
