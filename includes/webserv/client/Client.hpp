// TODO: don't forget header

#ifndef WEBSERV_CLIENT_CLIENT_HPP
#define WEBSERV_CLIENT_CLIENT_HPP

/**
 * @file Client.hpp
 * @brief [TODO:description]
 */

#include <ctime>
#include <cstring>
#include <string>
#include <common/common.hpp>
#include <webserv/types.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/client/Response.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/handler/ExecutionHandler.hpp>
#include <webserv/handler/RequestHandler.hpp>
#include <webserv/handler/ResponseHandler.hpp>

namespace webserv
{
namespace client
{

enum e_ClientStatus
{
	E_CLI_REQUEST,
	E_CLI_ERR_PARSING,
	E_CLI_DISCONNECTED,
};

class Client
{
	public:
		Client();
		Client(const t_SocketPairClient &client, const config::ServerConfig &serverConfig);
		~Client();

		Client(const Client &rhs);
		Client &operator=(const Client &rhs);

		static t_Logger					getLogger() ;

		const common::core::net::TcpClient	&getSocket() const;
		void								setSocket(const common::core::net::TcpClient &socket);
		sockaddr_storage					getSockaddrStorage() const;
		void								setSockaddrStorage(const sockaddr_storage &sockaddr_storage);
		e_ClientStatus						getStatus() const;
		void								setStatus(const e_ClientStatus status);
		handler::ExecutionHandler			&getExecutionHandler();
		handler::RequestHandler				&getRequestHandler();
		handler::ResponseHandler			&getResponseHandler();
		config::ServerConfig				&getServerConfig();
		HTTPError							&getHTTPError();
		void								setHTTPError(const HTTPError &error);
		std::time_t							getLastActivityTime() const;
		void								setLastActivityTime(const std::time_t time);
		std::time_t							getEffectiveKeepaliveTimeout() const;
		void								setEffectiveKeepaliveTimeout(const std::time_t timeout);

		void								receiveData();
		void								sendData();
		void								processHTTPCycle();

	private:
		t_Logger							_logger;
		int									_id;
		common::core::net::TcpClient		_socket;
		sockaddr_storage					_sockaddr_storage;
		e_ClientStatus						_status;
		config::ServerConfig				_serverConfig;
		handler::ExecutionHandler			_executionHandler;
		handler::RequestHandler				_requestHandler;
		handler::ResponseHandler			_responseHandler;
		HTTPError							_HTTPError;
		std::time_t							_lastActivityTime;
		std::time_t							_effectiveKeepaliveTimeout;
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_CLIENTNT_HPP
