// TODO: don't forget header

#ifndef WEBSERV_HANDLER_CLIENTHANDLER_HPP
#define WEBSERV_HANDLER_CLIENTHANDLER_HPP

/**
 * @file ClientHandler.hpp
 * @brief [TODO:description]
 */

#include <map>
#include <common/core/io/IEventIO.hpp>
#include <webserv/client/Client.hpp>

namespace webserv
{
namespace handler
{

class ClientHandler
{
	public:
		ClientHandler();
		~ClientHandler();

		ClientHandler(const ClientHandler &rhs);
		ClientHandler &operator=(const ClientHandler &rhs);

		void						addClient(Client &client);
		void						removeClient(Client &client);
		void						processClients(common::core::io::IEventIO ioMultiplexer);

	private:
		std::map<int, Client>		_clients;
		common::core::io::IEventIO	_ioMultiplexer;
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_CLIENTHANDLER_HPP
