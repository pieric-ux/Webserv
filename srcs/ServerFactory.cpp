// TODO: don't forget header

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

#include "log42/Logger.hpp"
#include <webserv/ServerFactory.hpp>

namespace webserv
{

static std::string	formatServerInfo(const config::ServerConfig &config);

/**
 * @brief [TODO:description]
 */
ServerFactory::ServerFactory() : _httpConfig(config::HTTPConfig::getInstance())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");
    _logger->setLevel(log42::logRecord::DEBUG);
    INFO(_logger, "ServerFactory created with default HTTPConfig instance");
}

/**
 * @brief [TODO:description]
 */
ServerFactory::ServerFactory(const config::HTTPConfig &httpConfig) : _httpConfig(httpConfig)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");
    _logger->setLevel(log42::logRecord::DEBUG);
}

/**
 * @brief [TODO:description]
 */
ServerFactory::~ServerFactory() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
ServerFactory::ServerFactory(const ServerFactory &rhs) : _logger(rhs._logger), _httpConfig(rhs._httpConfig) {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
ServerFactory &ServerFactory::operator=(const ServerFactory &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	ServerFactory::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");

}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Servers	ServerFactory::createServers() const
{
	t_Servers servers;
	t_ServerConfigs::const_iterator	it = _httpConfig.getServerConfigs().begin();

	for (; it != _httpConfig.getServerConfigs().end(); ++it)
	{
		servers.push_back(Server(*it));
		INFO(_logger, std::string("Created server with config: ") + formatServerInfo(*it));
	}
	
	std::ostringstream oss;
	oss << "Created " << servers.size() << " server(s) from config";
	INFO(_logger, oss.str());

	return servers;
}

/**
 * @brief [TODO:description]
 *
 * @param config [TODO:parameter]
 * @return [TODO:return]
 */
static std::string	formatServerInfo(const config::ServerConfig &config)
{
	std::ostringstream oss;

	oss << "listen : [";
	t_Listen::const_iterator lit = config.getListen().begin();
	for (; lit != config.getListen().end(); ++lit)
	{
		if (lit != config.getListen().begin())
			oss << ", ";
		oss << lit->address << ":" << lit->port;
	}
	oss << "] | server_name : [";

	t_Servernames::const_iterator sit = config.getServerNames().begin();
	for (; sit != config.getServerNames().end(); ++sit)
	{
		if (sit != config.getServerNames().begin())
			oss << ", ";
		oss << *sit;
	}
	oss << "]";
	return oss.str();
}

} // !webserv
