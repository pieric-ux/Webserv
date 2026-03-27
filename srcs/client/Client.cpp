// TODO: don't forget header

/**
 * @file Client.cpp
 * @brief [TODO:description]
 */

#include <webserv/client/Client.hpp>
#include <cstring>

namespace webserv
{
namespace client
{

/**
 * @brief [TODO:description]
 */
Client::Client()
	:	_id(-1),
		_socket(),
		_flags(static_cast<e_ClientFlags>(0)),
		_status(E_CLI_REQUEST),
		_executionHandler(),
		_requestHandler(),
		_responseHandler(),
		_serverConfig(),
		_HTTPError(),
		_lastActivityTime(0)
{
	std::memset(&_sockaddr_storage, 0, sizeof(_sockaddr_storage));
}

/**
 * @brief [TODO:description]
 *
 * @param socket [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
Client::Client(const common::core::net::TcpClient socket, const config::ServerConfig &serverConfig)
	:	_id(-1),
		_socket(socket),
		_flags(static_cast<e_ClientFlags>(0)),
		_status(E_CLI_REQUEST),
		_executionHandler(),
		_requestHandler(),
		_responseHandler(),
		_serverConfig(serverConfig),
		_HTTPError(status::StatusCode()),
		_lastActivityTime(0)
{
	std::memset(&_sockaddr_storage, 0, sizeof(_sockaddr_storage));
}

/**
 * @brief [TODO:description]
 */
Client::~Client() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Client::Client(const Client &rhs)
	:	_id(rhs._id),
		_socket(rhs._socket),
		_sockaddr_storage(rhs._sockaddr_storage),
		_flags(rhs._flags),
		_status(rhs._status),
		_executionHandler(rhs._executionHandler),
		_requestHandler(rhs._requestHandler),
		_responseHandler(rhs._responseHandler),
		_serverConfig(rhs._serverConfig),
		_HTTPError(rhs._HTTPError),
		_lastActivityTime(rhs._lastActivityTime)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
Client &Client::operator=(const Client &rhs)
{
	if (this != &rhs)
	{
		_id = rhs._id;
		_socket = rhs._socket;
		_sockaddr_storage = rhs._sockaddr_storage;
		_flags = rhs._flags;
		_status = rhs._status;
		_executionHandler = rhs._executionHandler;
		_requestHandler = rhs._requestHandler;
		_responseHandler = rhs._responseHandler;
		_serverConfig = rhs._serverConfig;
		_HTTPError = rhs._HTTPError;
		_lastActivityTime = rhs._lastActivityTime;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	Client::getLogger() const
{
	return _logger;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
e_ClientFlags Client::getFlags() const
{
	return _flags;
}

/**
 * @brief [TODO:description]
 *
 * @param flags [TODO:parameter]
 */
void Client::setFlags(const e_ClientFlags flags)
{
	_flags = flags;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
e_ReceiveDataStatus Client::getStatus() const
{
	return _status;
}

/**
 * @brief [TODO:description]
 *
 * @param status [TODO:parameter]
 */
void Client::setStatus(const e_ReceiveDataStatus status)
{
	_status = status;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
handler::ExecutionHandler &Client::getExecutionHandler()
{
	return _executionHandler;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
handler::RequestHandler &Client::getRequestHandler()
{
	return _requestHandler;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
handler::ResponseHandler &Client::getResponseHandler()
{
	return _responseHandler;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
config::ServerConfig &Client::getServerConfig()
{
	return _serverConfig;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
HTTPError &Client::getHTTPError()
{
	return _HTTPError;
}

/**
 * @brief [TODO:description]
 *
 * @param error [TODO:parameter]
 */
void Client::setHTTPError(const HTTPError &error)
{
	_HTTPError = error;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int Client::getLastActivityTime() const
{
	return _lastActivityTime;
}

/**
 * @brief [TODO:description]
 *
 * @param time [TODO:parameter]
 */
void Client::setLastActivityTime(const int time)
{
	_lastActivityTime = time;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
e_ReceiveDataStatus	Client::receiveData()
{
	return E_CLI_REQUEST;
}

/**
 * @brief [TODO:description]
 */
void Client::sendData()
{

}

/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 */
void Client::prepareHeadersRequest(const t_raw &buffer)
{
	(void)buffer;
}

/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 */
void Client::prepareBodyRequest(const t_raw &buffer)
{
	(void)buffer;
}

/**
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param responseHandler [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
void Client::prepareExecution(handler::RequestHandler requestHandler, handler::ResponseHandler responseHandler, config::ServerConfig serverConfig)
{
	(void)requestHandler;
	(void)responseHandler;
	(void)serverConfig;
}

/**
 * @brief [TODO:description]
 */
void Client::prepareHeadersResponse()
{

}

/**
 * @brief [TODO:description]
 */
void Client::prepareBodyResponse()
{

}

} // !client
} // !webserv
