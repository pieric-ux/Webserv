/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerFactory.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file ServerFactory.cpp
 * @brief Implements ServerFactory, which builds Server instances from the server configurations held in an HTTPConfig.
 */

#include "log42/Logger.hpp"
#include <webserv/ServerFactory.hpp>

namespace webserv
{

static std::string	formatServerInfo(const config::ServerConfig &config);

/**
 * @brief Constructs a ServerFactory bound to the singleton HTTPConfig instance and sets up its logger.
 */
ServerFactory::ServerFactory() : _httpConfig(config::HTTPConfig::getInstance())
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");
    _logger->setLevel(log42::logRecord::DEBUG);
    INFO(_logger, "ServerFactory created with default HTTPConfig instance");
}

/**
 * @brief Constructs a ServerFactory bound to the given HTTPConfig and sets up its logger.
 */
ServerFactory::ServerFactory(const config::HTTPConfig &httpConfig) : _httpConfig(httpConfig)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");
    _logger->setLevel(log42::logRecord::DEBUG);
}

/**
 * @brief Destroys the ServerFactory; holds no owned resources to release.
 */
ServerFactory::~ServerFactory() {}

/**
 * @brief Copy-constructs a ServerFactory, sharing the source's logger and HTTPConfig reference.
 *
 * @param rhs The ServerFactory to copy from.
 */
ServerFactory::ServerFactory(const ServerFactory &rhs) : _logger(rhs._logger), _httpConfig(rhs._httpConfig) {}

/**
 * @brief Copy-assigns from another ServerFactory, copying its logger while leaving the bound HTTPConfig reference unchanged.
 *
 * @param rhs The ServerFactory to assign from.
 * @return Reference to this ServerFactory.
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
 * @brief Returns the logger associated with the ServerFactory module.
 *
 * @return The "webserv.serverfactory" logger obtained from the logging manager.
 */
t_Logger	ServerFactory::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.serverfactory");

}

/**
 * @brief Builds one Server per server configuration in the bound HTTPConfig, logging each created server and the total count.
 *
 * @return The collection of Server instances created from the configuration.
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
 * @brief Formats a server configuration's listen endpoints and server names into a human-readable summary string for logging.
 *
 * @param config The server configuration whose listen directives and server names are summarized.
 * @return A string of the form "listen : [addr:port, ...] | server_name : [name, ...]".
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
