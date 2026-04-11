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
ClientHandler::ClientHandler() : _clients(), _ioMultiplexer()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.clienthandler");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ClientHandler instance created");
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
* @param ioMultiplexer [TODO:parameter]
 */
void	ClientHandler::setIoMultiplexer(const t_ioMultiplexer &ioMultiplexer)
{
	_ioMultiplexer = ioMultiplexer;
}

/**
 * @brief [TODO:description]
 *
 * @param client [TODO:parameter]
 */
void	ClientHandler::addClient(const t_SocketPairClient &client, const config::ServerConfig &serverConfig)
{
	try{
		_ioMultiplexer->add(client.first.getFd(), static_cast<common::core::io::IEventIO::e_Event>(common::core::io::IEventIO::E_IN | common::core::io::IEventIO::E_OUT));
	} catch (const std::exception &e) {
		ERROR(_logger, "Failed to add client fd to io multiplexer: " + std::string(e.what()));
		throw;
	}

	try{
		t_AddrPortPair addr = common::core::net::getNameInfo(client.second);
		INFO(_logger, "Added client " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(client.first.getFd()));
	} catch (const std::exception &e) {
		WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
	}
	_clients.insert(std::make_pair(client.first.getFd(), client::Client(client, serverConfig)));
}

/**
 * @brief [TODO:description]
 *
 * @param client [TODO:parameter]
 */
t_Clients::iterator	ClientHandler::removeClient(client::Client &client)
{
	try{
		_ioMultiplexer->remove(client.getSocket().getFd());
	} catch (const std::exception &e) {
		ERROR(_logger, "Failed to remove client fd from io multiplexer: " + std::string(e.what()));
	}

	t_Clients::iterator it = _clients.find(client.getSocket().getFd());
	if (it != _clients.end())
	{
		try{
			t_AddrPortPair addr = common::core::net::getNameInfo(client.getSockaddrStorage());
			INFO(_logger, "Removed client " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(client.getSocket().getFd()));
		} catch (const std::exception &e) {
			WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
		}
		return _clients.erase(it);
	}
	else
	{
		WARNING(_logger, "Attempted to remove non-existent client fd=" + common::core::utils::toString(client.getSocket().getFd()));
		return _clients.end();
	}
}

/**
 * @brief [TODO:description]
 *
 * @param ioMultiplexer [TODO:parameter]
 */
void	ClientHandler::processClients()
{
	//DEBUG(_logger, "Processing clients (" + common::core::utils::toString(_clients.size()) + " active)");

	std::time_t now = std::time(NULL);

	t_Clients::iterator it = _clients.begin();
	while (it != _clients.end())
	{
		client::Client &client = it->second;
		std::string clientAddr;
		try {
			t_AddrPortPair addr = common::core::net::getNameInfo(client.getSockaddrStorage());
			clientAddr = addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(it->first);
		} catch (const std::exception &e) {
			WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
		}

		if (client.getEffectiveKeepaliveTimeout() > 0 &&
			now - client.getLastActivityTime() > client.getEffectiveKeepaliveTimeout())
		{
			INFO(_logger, "Client " + clientAddr + " timed out");
			it = removeClient(client);
			continue;
		}

		int events = _ioMultiplexer->getEvents(client.getSocket().getFd());
		if (client.getStatus() != client::E_CLI_ERR_PARSING)
		{
			if (events & common::core::io::IEventIO::E_IN)
			{
				client.setLastActivityTime(now);
				client.receiveData();
			}
			if (client.getStatus() == client::E_CLI_DISCONNECTED)
			{
				it = removeClient(client);
				continue;
			}
			//DEBUG(_logger, "buffer size: " + common::core::utils::toString(client.getRequestHandler().getBufferRequest().size()));
			//DEBUG(_logger, "request flags: 0x" + common::core::utils::toString(client.getRequestHandler().getRequest().getFlags()));
			//DEBUG(_logger, "exec flags: 0x" + common::core::utils::toString(client.getExecutionHandler().getFlags()));
			if ((client.getRequestHandler().getBufferRequest().size() > 0
					|| (client.getRequestHandler().getRequest().getFlags() & client::E_REQ_HEADERS_VALIDATED))
				&& !(client.getExecutionHandler().getFlags() & E_EXEC_COMPLETE))
			{
				DEBUG(_logger, "Processing HTTPCycle for " + clientAddr);
				client.processHTTPCycle();
			}
		}
		else
		{
			DEBUG(_logger, "Client " + clientAddr + " in error state, building error response");
			client.buildErrorResponse();
		}
		if (events & common::core::io::IEventIO::E_OUT)
			client.sendData();
		++it;
	}
}

} // !handler
} // !webserv
