// TODO: don't forget header

#ifndef WEBSERV_HANDLER_CLIENTHANDLER_HPP
#define WEBSERV_HANDLER_CLIENTHANDLER_HPP

/**
 * @file ClientHandler.hpp
 * @brief [TODO:description]
 */

#include <webserv/client/Client.hpp>
#include <webserv/types.hpp>

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

		static t_Logger	getLogger();

		void			setIoMultiplexer(const t_ioMultiplexer &ioMultiplexer);

		void			addClient(const t_SocketPairClient &client);
		void			removeClient(client::Client &client);
		void			processClients();

	private:
		t_Logger		_logger;
		t_Clients		_clients;
		t_ioMultiplexer	_ioMultiplexer;
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_CLIENTHANDLER_HPP
