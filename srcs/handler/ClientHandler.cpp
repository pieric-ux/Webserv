/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file ClientHandler.cpp
 * @brief Implements ClientHandler, which tracks connected clients and drives their HTTP processing through the I/O multiplexer.
 */

#include <webserv/handler/ClientHandler.hpp>

namespace webserv
{
namespace handler
{

/**
 * @brief Constructs an empty ClientHandler and initializes its logger.
 */
ClientHandler::ClientHandler() : _clients(), _ioMultiplexer()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.clienthandler");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ClientHandler instance created");
}

/**
 * @brief Destroys the ClientHandler.
 */
ClientHandler::~ClientHandler() {}

/**
 * @brief Copy-constructs a ClientHandler from another instance.
 *
 * @param rhs The ClientHandler whose logger and client map are copied.
 */
ClientHandler::ClientHandler(const ClientHandler &rhs) : _logger(rhs._logger), _clients(rhs._clients) {}

/**
 * @brief Copy-assigns the client map and logger from another ClientHandler.
 *
 * @param rhs The ClientHandler to copy from.
 * @return Reference to this instance.
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
 * @brief Returns the logger associated with this class.
 *
 * @return The "webserv.handler.clienthandler" logger from the log42 manager.
 */
t_Logger	ClientHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.clienthandler");
}

/**
 * @brief Sets the I/O multiplexer used to register and monitor client sockets.
 *
* @param ioMultiplexer The shared event-based I/O multiplexer to use.
 */
void	ClientHandler::setIoMultiplexer(const t_ioMultiplexer &ioMultiplexer)
{
	_ioMultiplexer = ioMultiplexer;
}

/**
 * @brief Registers a newly accepted client: adds its socket to the I/O multiplexer for read/write events and inserts a Client into the map.
 *
 * @param client The accepted socket paired with its peer address storage.
 * @param serverConfig The server configuration that governs this client.
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
	int fd_before = client.first.getFd();
	DEBUG(_logger, "addClient: fd before insert=" + common::core::utils::toString(fd_before));
	_clients.insert(std::make_pair(client.first.getFd(), client::Client(client, serverConfig, _ioMultiplexer)));
	DEBUG(_logger, "addClient: fd after  insert=" + common::core::utils::toString(fd_before)
		+ " map_key=" + common::core::utils::toString(_clients.rbegin()->first)
		+ " socket_in_map=" + common::core::utils::toString(_clients.rbegin()->second.getSocket().getFd()));
}

/**
 * @brief Removes a client: unregisters its socket from the I/O multiplexer and erases it from the map, returning the next iterator.
 *
 * @param client The client to remove.
 * @return Iterator to the element following the erased one, or end() if the client was not found in the map.
 */
t_Clients::iterator	ClientHandler::removeClient(client::Client &client)
{
	try{
		_ioMultiplexer->remove(client.getSocket().getFd());
	} catch (const std::exception &e) {
		ERROR(_logger, "Failed to remove client fd from io multiplexer: " + std::string(e.what()));
	}

	if (client.isCgiRoute())
		client.getExecutionHandler().getCgi().reset();

	t_Clients::iterator it = _clients.find(client.getSocket().getFd());
	if (it != _clients.end())
	{
		try{
			t_AddrPortPair addr = common::core::net::getNameInfo(client.getSockaddrStorage());
			INFO(_logger, "Removed client " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(client.getSocket().getFd()));
		} catch (const std::exception &e) {
			WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
		}
		t_Clients::iterator nextIt = it;
		++nextIt;
		_clients.erase(it);
		return nextIt;
	}
	else
	{
		WARNING(_logger, "Attempted to remove non-existent client fd=" + common::core::utils::toString(client.getSocket().getFd()));
		return _clients.end();
	}
}

/**
 * @brief Iterates over all connected clients, expiring timed-out ones, reading and sending data, driving the HTTP/CGI cycle and resetting completed connections.
 */
void	ClientHandler::processClients()
{
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

			if (client.isCgiRoute())
			{
				DEBUG(_logger, "Driving CGI IO for " + clientAddr);
				client.driveCgiIO();
			}
			bool hasReqData = client.getRequestHandler().getBufferRequest().size() > 0
				|| (client.getRequestHandler().getRequest().getFlags() & client::E_REQ_HEADERS_VALIDATED);
			bool respFullySent = (client.getExecutionHandler().getFlags() & E_EXEC_COMPLETE)
				&& (client.getResponseHandler().getResponse().getFlags() & client::E_RESP_HEADERS_SENT);
			if (hasReqData && !respFullySent)
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

		
		bool execComplete = client.getExecutionHandler().getFlags() & handler::E_EXEC_COMPLETE;
		bool bufferDrained = client.getResponseHandler().getBufferResponse().empty();
		bool expectPending = client.getRequestHandler().getParser().getFlags() & parser::E_PARS_EXPECT;
		if ((execComplete && bufferDrained) || expectPending)
			client.resetAll();

		++it;
	}
}

} // !handler
} // !webserv
