// TODO: don't forget header

/**
 * @file Client.cpp
 * @brief [TODO:description]
 */

#include "webserv/client/Response.hpp"
#include <webserv/client/Client.hpp>

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
		_status(E_CLI_REQUEST),
		_executionHandler(),
		_requestHandler(_serverConfig),
		_responseHandler(),
		_serverConfig(),
		_HTTPError(),
		_lastActivityTime(0),
		_effectiveKeepaliveTimeout(0)
{
	std::memset(&_sockaddr_storage, 0, sizeof(_sockaddr_storage));
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.client");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Client instance created with default constructor");
}

/**
 * @brief [TODO:description]
 *
 * @param socket [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 */
Client::Client(const t_SocketPairClient &client, const config::ServerConfig &serverConfig)
	:	_id(-1),
		_socket(client.first),
		_sockaddr_storage(client.second),
		_status(E_CLI_REQUEST),
		_executionHandler(),
		_requestHandler(_serverConfig),
		_responseHandler(),
		_serverConfig(serverConfig),
		_HTTPError(),
		_lastActivityTime(std::time(NULL)),
		_effectiveKeepaliveTimeout(static_cast<std::time_t>(serverConfig.getKeepAliveTimeout()))
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.client");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Client instance created with socket and server configuration");
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
	:	_logger(rhs._logger),
		_id(rhs._id),
		_socket(rhs._socket),
		_sockaddr_storage(rhs._sockaddr_storage),
		_status(rhs._status),
		_executionHandler(rhs._executionHandler),
		_requestHandler(rhs._requestHandler),
		_responseHandler(rhs._responseHandler),
		_serverConfig(rhs._serverConfig),
		_HTTPError(rhs._HTTPError),
		_lastActivityTime(rhs._lastActivityTime),
		_effectiveKeepaliveTimeout(rhs._effectiveKeepaliveTimeout)
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
		_logger = rhs._logger;
		_id = rhs._id;
		_socket = rhs._socket;
		_sockaddr_storage = rhs._sockaddr_storage;
		_status = rhs._status;
		_executionHandler = rhs._executionHandler;
		_requestHandler = rhs._requestHandler;
		_responseHandler = rhs._responseHandler;
		_serverConfig = rhs._serverConfig;
		_HTTPError = rhs._HTTPError;
		_lastActivityTime = rhs._lastActivityTime;
		_effectiveKeepaliveTimeout = rhs._effectiveKeepaliveTimeout;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	Client::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.client");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const common::core::net::TcpClient &Client::getSocket() const
{
	return _socket;
}

/**
 * @brief [TODO:description]
 *
 * @param socket [TODO:parameter]
 */
void Client::setSocket(const common::core::net::TcpClient &socket)
{
	_socket = socket;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
sockaddr_storage Client::getSockaddrStorage() const
{
	return _sockaddr_storage;
}

/**
 * @brief [TODO:description]
 *
 * @param sockaddr_storage [TODO:parameter]
 */
void Client::setSockaddrStorage(const sockaddr_storage &sockaddr_storage)
{
	_sockaddr_storage = sockaddr_storage;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
e_ClientStatus Client::getStatus() const
{
	return _status;
}

/**
 * @brief [TODO:description]
 *
 * @param status [TODO:parameter]
 */
void Client::setStatus(const e_ClientStatus status)
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
std::time_t Client::getLastActivityTime() const
{
	return _lastActivityTime;
}

/**
 * @brief [TODO:description]
 *
 * @param time [TODO:parameter]
 */
void Client::setLastActivityTime(const std::time_t time)
{
	_lastActivityTime = time;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::time_t Client::getEffectiveKeepaliveTimeout() const
{
	return _effectiveKeepaliveTimeout;
}

/**
 * @brief [TODO:description]
 *
 * @param timeout [TODO:parameter]
 */
void Client::setEffectiveKeepaliveTimeout(const std::time_t timeout)
{
	_effectiveKeepaliveTimeout = timeout;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
void	Client::receiveData()
{
	unsigned char	buf[4096];
	ssize_t			rd;

	try
	{
		rd = _socket.recv(buf, sizeof(buf));
	}
	catch (const std::exception &e)
	{
		try {
			t_AddrPortPair addr = common::core::net::getNameInfo(this->getSockaddrStorage());
			WARNING(_logger, "recv error from " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(this->getSocket().getFd()) + ": " + std::string(e.what()));
		} catch (const std::exception &e) {
			WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
		}
		_status = E_CLI_DISCONNECTED;
		return ;
	}
	if (rd == 0)
	{
		try {
			t_AddrPortPair addr = common::core::net::getNameInfo(this->getSockaddrStorage());
			DEBUG(_logger, "Client disconnected " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(this->getSocket().getFd()));
		} catch (const std::exception &e) {
			WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
		}
		_status = E_CLI_DISCONNECTED;
		return ;
	}
	t_raw raw(buf, buf + rd);
	DEBUG(_logger, "Received " + common::core::utils::toString(rd) + " bytes from fd=" + common::core::utils::toString(this->getSocket().getFd()));
	_requestHandler.appendToBufferRequest(raw);
}

/**
 * @brief [TODO:description]
 */
void Client::sendData()
{

}


/**
 * @brief [TODO:description]
 */
void Client::processHTTPCycle()
{
	int method = _requestHandler.getRequest().getMethod();
	if (!(_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED))
		try{
			_requestHandler.parseHeaders();
		} catch (const HTTPError &e) {
			INFO(_logger, "While parsing request headers: " + std::string(e.what()));
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	if (_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED && (method == config::POST || method == config::PUT || method == config::DELETE))
		try{
			_requestHandler.parseBody();
		} catch (const HTTPError &e) {
			INFO(_logger, "While parsing request body: " + std::string(e.what()));
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	if (_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED)
		try{
			_executionHandler.execute(_requestHandler, _responseHandler, _serverConfig);
		} catch (const HTTPError &e) {
			INFO(_logger, "While executing request: " + std::string(e.what()));
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	if (_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED && !(_responseHandler.getResponse().getFlags() & E_RESP_HEADERS_SENT))
		try{
			_responseHandler.buildHeadersResponse(_requestHandler.getRequest());
		} catch (const HTTPError &e) {
			INFO(_logger, "While preparing response headers: " + std::string(e.what()));
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	if (_responseHandler.getResponse().getFlags() & E_RESP_HEADERS_SENT && _responseHandler.getResponse().getBody().size() > 0)
		try{
			_responseHandler.buildBodyResponse();
		} catch (const HTTPError &e) {
			INFO(_logger, "While preparing response body: " + std::string(e.what()));
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
}

} // !client
} // !webserv
