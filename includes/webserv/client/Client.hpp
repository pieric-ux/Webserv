// TODO: don't forget header

#ifndef WEBSERV_CLIENT_CLIENT_HPP
#define WEBSERV_CLIENT_CLIENT_HPP

/**
 * @file Client.hpp
 * @brief [TODO:description]
 */

#include <string>
#include <common/core/net/sockets/TcpClient.hpp>
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

enum e_ClientFlags
{
	E_CLI_ERR_PARSING = 1 << 0,
};

enum e_ReceiveDataStatus
{
	E_CLI_DISCONNECTED,
	E_CLI_REQUEST,
};

class Client
{
	public:
		Client();
		Client(const common::core::net::TcpClient socket, const config::ServerConfig &serverConfig);
		~Client();

		Client(const Client &rhs);
		Client &operator=(const Client &rhs);

		e_ClientFlags					getFlags() const;
		void							setFlags(const e_ClientFlags flags);
		e_ReceiveDataStatus				getStatus() const;
		void							setStatus(const e_ReceiveDataStatus status);
		handler::ExecutionHandler		&getExecutionHandler();
		handler::RequestHandler			&getRequestHandler();
		handler::ResponseHandler		&getResponseHandler();
		config::ServerConfig			&getServerConfig();
		HTTPError						&getHTTPError();
		void							setHTTPError(const HTTPError &error);
		int								getLastActivityTime() const;
		void							setLastActivityTime(const int time);

		e_ReceiveDataStatus				receiveData();
		void							sendData();

	private:
		int								_id;
		common::core::net::TcpClient	_socket;
		sockaddr_storage				_sockaddr_storage;
		e_ClientFlags					_flags;
		e_ReceiveDataStatus				_status;
		handler::ExecutionHandler		_executionHandler;
		handler::RequestHandler			_requestHandler;
		handler::ResponseHandler		_responseHandler;
		config::ServerConfig			_serverConfig;
		HTTPError						_HTTPError;
		int								_lastActivityTime;

		void							prepareHeadersRequest(const std::string &buffer);
		void							prepareBodyRequest(const std::string &buffer);
		void							prepareExecution(handler::RequestHandler requestHandler, handler::ResponseHandler responseHandler, config::ServerConfig serverConfig);
		void							prepareHeadersResponse();
		void							prepareBodyResponse();
};

} // !client
} // !webserv

#endif // !WEBSERV_CLIENT_CLIENTNT_HPP
