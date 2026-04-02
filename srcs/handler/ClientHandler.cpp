// TODO: don't forget header

/**
 * @file ClientHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/ClientHandler.hpp>

namespace webserv
{
namespace handler
{

/**
 * @brief [TODO:description]
 */
ClientHandler::ClientHandler() : _clients()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.clienthandler");
	_logger->setLevel(log42::logRecord::INFO);
}

/**
 * @brief [TODO:description]
 */
ClientHandler::~ClientHandler() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
ClientHandler::ClientHandler(const ClientHandler &rhs) : _logger(rhs._logger), _clients(rhs._clients) {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
ClientHandler &ClientHandler::operator=(const ClientHandler &rhs)
{
	if (this != &rhs)
	{
		_clients = rhs._clients;
		_logger = rhs._logger;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	ClientHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.clienthandler");
}

/**
 * @brief [TODO:description]
 *
 * @param client [TODO:parameter]
 */
void ClientHandler::addClient(client::Client &client)
{
	(void)client;
}

/**
 * @brief [TODO:description]
 *
 * @param client [TODO:parameter]
 */
void ClientHandler::removeClient(client::Client &client)
{
	(void)client;
}

/**
 * @brief [TODO:description]
 *
 * @param ioMultiplexer [TODO:parameter]
 */
void ClientHandler::processClients(t_ioMultiplexer ioMultiplexer)
{
	(void)ioMultiplexer;
}

} // !handler
} // !webserv
