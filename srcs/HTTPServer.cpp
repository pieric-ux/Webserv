// TODO: don't forget header

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

#include <webserv/HTTPServer.hpp>
#include <webserv/session/SessionStore.hpp>

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
							_statusCodeRegistry(status::StatusCodeRegistry::getInstance())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv");
	_logger->setLevel(log42::logRecord::DEBUG);
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
		_statusCodeRegistry(rhs._statusCodeRegistry)
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
 *
 * @return [TODO:return]
 */
t_Logger HTTPServer::getLogger() const
{
	return _logger;
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
			_ioMultiplexer->add(socketIt->first.getFd(), common::core::io::IEventIO::E_IN);
			INFO(_logger, "Registered fd=" + common::core::utils::toString(socketIt->first.getFd()) + " to I/O multiplexer");
			try {
				t_AddrPortPair addr = common::core::net::getNameInfo(socketIt->second);
				INFO(_logger, "Listening on " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(socketIt->first.getFd()));
			} catch (const std::exception &e) {
				WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
			}
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
void	HTTPServer::run()
{
	INFO(_logger, "HTTPServer is running...");

	while (!g_SignalStatus)
	{
		int ready;

		try {
			ready = _ioMultiplexer->wait(IO_TIMEOUT_MS);
			if (!ready)
				continue;
		} catch (const std::exception &e) {
			ERROR(_logger, "I/O multiplexer wait failed: " + std::string(e.what()));
		}

		t_Servers::const_iterator it = _servers.begin();
		for (; it != _servers.end(); ++it)
		{
    		const t_ServerSockets &sockets = it->getSockets();
			t_ServerSockets::const_iterator socketIt = sockets.begin();
			for (; socketIt != sockets.end(); ++socketIt)
			{
				if (_ioMultiplexer->getEvents(socketIt->first.getFd()) & common::core::io::IEventIO::E_IN)
				{
					try {
						t_AddrPortPair addr = common::core::net::getNameInfo(socketIt->second);
						INFO(_logger, "Incoming connection on " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(socketIt->first.getFd()));
					} catch (const std::exception &e) {
						WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
					}

					try {
						connectClient(*socketIt, it->getConfig());
					} catch (const std::exception &e) {
						continue;
					}
				}
			}
		}
		try {
			_clientHandler.processClients();
		} catch (const std::exception &e) {
			ERROR(_logger, "processClients() failed: " + std::string(e.what()));
		} catch (...) {
			ERROR(_logger, "processClients() failed: unknown exception");
		}

		static std::time_t lastPurge = std::time(NULL);
		std::time_t now = std::time(NULL);
		if (now - lastPurge >= SESSION_PURGE_INTERVAL_S)
		{
			session::SessionStore::getInstance().purgeExpired();
			lastPurge = now;
		}
	}
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
void HTTPServer::connectClient(const t_SocketPairServer &socket, const config::ServerConfig &serverConfig)
{
	try {
		t_SocketPairClient client = socket.first.accept<sockaddr_storage>();
			try{
				t_AddrPortPair addr = common::core::net::getNameInfo(client.second);
				INFO(_logger, "Accepted new client connection on " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(client.first.getFd()));
			} catch (const std::exception &e) {
				WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
			}
		_clientHandler.addClient(client, serverConfig);
	} catch (const std::exception &e) {
		ERROR(_logger, "Failed connect client connection: " + std::string(e.what()));
		throw;
	}
}

} // !webserv
