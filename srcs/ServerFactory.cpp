// TODO: don't forget header

/**
 * @file HTTPServer.hpp
 * @brief [TODO:description]
 */

#include <webserv/ServerFactory.hpp>

namespace webserv
{

/**
 * @brief [TODO:description]
 */
ServerFactory::ServerFactory() : _httpConfig(config::HTTPConfig::getInstance())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");
    _logger->setLevel(log42::logRecord::INFO);
}

/**
 * @brief [TODO:description]
 */
ServerFactory::ServerFactory(const config::HTTPConfig &httpConfig) : _httpConfig(httpConfig)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");
    _logger->setLevel(log42::logRecord::INFO);
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
	}

	return servers;
}

} // !webserv
