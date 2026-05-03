// TODO: don't forget header

/**
 * @file Client.cpp
 * @brief [TODO:description]
 */

#include "webserv/parser/Parser.hpp"
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
		_serverConfig(),

		_executionHandler(),
		_requestHandler(_serverConfig),
		_responseHandler(),
		_HTTPError(),
		_lastActivityTime(0),
		_effectiveKeepaliveTimeout(0),
		_ioMultiplexer()
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
		_serverConfig(serverConfig),
		_executionHandler(),
		_requestHandler(_serverConfig),
		_responseHandler(),
		_HTTPError(),
		_lastActivityTime(std::time(NULL)),
		_effectiveKeepaliveTimeout(static_cast<std::time_t>(serverConfig.getKeepAliveTimeout())),
		_ioMultiplexer()
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
		_serverConfig(rhs._serverConfig),
		_executionHandler(rhs._executionHandler),
		_requestHandler(_serverConfig),
		_responseHandler(rhs._responseHandler),
		_HTTPError(rhs._HTTPError),
		_lastActivityTime(rhs._lastActivityTime),
		_effectiveKeepaliveTimeout(rhs._effectiveKeepaliveTimeout),
		_ioMultiplexer(rhs._ioMultiplexer)
{
	_requestHandler = rhs._requestHandler;
}

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
		_ioMultiplexer = rhs._ioMultiplexer;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @param client [TODO:parameter]
 * @param serverConfig [TODO:parameter]
 * @param mux [TODO:parameter]
 */
void Client::init(const t_SocketPairClient &client,
		const config::ServerConfig &serverConfig,
		const t_ioMultiplexer &mux)
{
	_socket = client.first;
	_sockaddr_storage = client.second;
	_id = _socket.getFd();
	_serverConfig = serverConfig;
	_status = E_CLI_REQUEST;
	_lastActivityTime = std::time(NULL);
	_effectiveKeepaliveTimeout = static_cast<std::time_t>(serverConfig.getKeepAliveTimeout());
	_ioMultiplexer = mux;
	INFO(_logger, "Client initialized in place fd=" + common::core::utils::toString(_id));
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
	unsigned char	buf[config::DefaultConfig::BUFFER_SIZE];
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
	const t_raw &buf = _responseHandler.getBufferResponse();
	if (buf.empty())
		return;

	ssize_t sd;
	try
	{
		sd = _socket.send(&buf[0], buf.size());
	}
	catch (const std::exception &e)
	{
		try {
			t_AddrPortPair addr = common::core::net::getNameInfo(this->getSockaddrStorage());
			WARNING(_logger, "send error to " + addr.first + ":" + addr.second + " fd=" + common::core::utils::toString(this->getSocket().getFd())
				+ ": " + std::string(e.what()));
		} catch (const std::exception &e) {
			WARNING(_logger, "Failed to get socket address info: " + std::string(e.what()));
		}
		_status = E_CLI_DISCONNECTED;
		return;
	}

	DEBUG(_logger, "Sent " + common::core::utils::toString(sd) + " bytes to fd="
		+ common::core::utils::toString(this->getSocket().getFd()));

	_responseHandler.eraseBufferResponseFront(static_cast<std::size_t>(sd));

	int flags = _responseHandler.getResponse().getFlags();
	if (!(flags & E_RESP_HEADERS_SENT))
		_responseHandler.getResponse().setFlags(flags | E_RESP_HEADERS_SENT);
}

/**
 * @brief [TODO:description]
 */
void Client::processHTTPCycle()
{
	if (!(_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED))
		try{
			_requestHandler.parseHeaders();
		} catch (const HTTPError &e) {
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	int method = _requestHandler.getRequest().getMethod();
	if (_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED &&
			(method == config::POST || method == config::PUT || method == config::DELETE) &&
			!(_requestHandler.getParser().getFlags() & parser::E_PARS_EXPECT))
		try{
			_requestHandler.parseBody();
		} catch (const HTTPError &e) {
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	if (_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED &&
			!(_requestHandler.getParser().getFlags() & parser::E_PARS_EXPECT) &&
			!isCgiRoute() &&
			!(_executionHandler.getFlags() & handler::E_EXEC_COMPLETE))
		try{
			_executionHandler.execute(_requestHandler, _responseHandler, _requestHandler.getRequest().getLocationConfig());
		} catch (const HTTPError &e) {
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
	method = _requestHandler.getRequest().getMethod();
	int parserFlags = _requestHandler.getParser().getFlags();
	bool cgiNotReady = isCgiRoute() && !(_executionHandler.getFlags() & handler::E_EXEC_COMPLETE);
	if (_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED &&
			!(_responseHandler.getResponse().getFlags() & E_RESP_HEADERS_SENT) &&
			((_executionHandler.getFlags() & handler::E_EXEC_COMPLETE)
				|| ((method == config::GET || method == config::HEAD) && !cgiNotReady)
				|| (parserFlags & parser::E_PARS_EXPECT)))
		try{
			_responseHandler.buildHeadersResponse(_requestHandler.getRequest(), _executionHandler.getFlags(), _requestHandler.getParser().getFlags());
			if (isCgiRoute()
					&& (_executionHandler.getFlags() & handler::E_EXEC_COMPLETE)
					&& !_executionHandler.getCgi().isPushed())
			{
				DEBUG(_logger, "processHTTPCycle: pushing CGI body after headers");
				_executionHandler.getCgi().pushBody(_responseHandler);
			}
		} catch (const HTTPError &e) {
			setHTTPError(e);
			setStatus(E_CLI_ERR_PARSING);
			return ;
		}
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool Client::isCgiRoute() const
{
	if (!(_requestHandler.getRequest().getFlags() & E_REQ_HEADERS_VALIDATED))
		return false;
	const config::LocationConfig &loc = _requestHandler.getRequest().getLocationConfig();
	if (!loc.isEnableCGI())
		return false;
	std::string ext = "." + handler::ExecutionHandler::getFileExtension(_requestHandler.getRequest().getAbsolutePath());
	return loc.getCgiExtensions().find(ext) != loc.getCgiExtensions().end();
}

/**
 * @brief [TODO:description]
 */
void Client::driveCgiIO()
{
	if (!isCgiRoute())
		return ;

	try
	{
		if (!(_executionHandler.getFlags() & handler::E_EXEC_COMPLETE))
		{
			_executionHandler.executeCGI(_requestHandler, _responseHandler,
				_requestHandler.getRequest().getLocationConfig(),
				*this, _ioMultiplexer);
		}

		if ((_executionHandler.getFlags() & handler::E_EXEC_COMPLETE)
				&& (_responseHandler.getResponse().getFlags() & E_RESP_HEADERS_SENT)
				&& !_executionHandler.getCgi().isPushed())
		{
			DEBUG(_logger, "driveCgiIO: pushing CGI body to response buffer");
			_executionHandler.getCgi().pushBody(_responseHandler);
		}
	}
	catch (const HTTPError &e)
	{
		ERROR(_logger, "driveCgiIO: " + std::string(e.what()));
		setHTTPError(e);
		setStatus(E_CLI_ERR_PARSING);
	}
}

/**
 * @brief Builds the HTTP error response from the stored HTTPError.
 */
void Client::buildErrorResponse()
{
	if (_responseHandler.getResponse().getFlags() & E_RESP_HEADERS_SENT)
		return ;
	DEBUG(_logger, "Building error response: " + std::string(_HTTPError.what()));
	_responseHandler.buildErrorResponse(_requestHandler.getRequest(), _HTTPError);
	_executionHandler.setFlags(_executionHandler.getFlags() | handler::E_EXEC_COMPLETE);
	DEBUG(_logger, "E_EXEC_COMPLETE flag set in execution handler after building error response");
}

/**
 * @brief [TODO:description]
 */
void Client::resetAll()
{
	DEBUG(_logger, "Client state reset for next request. Effective keep-alive timeout set to " + common::core::utils::toString(_effectiveKeepaliveTimeout) + " seconds");

	if (_requestHandler.getParser().getFlags() & parser::E_PARS_EXPECT)
	{
		_requestHandler.getParser().setFlags(_requestHandler.getParser().getFlags() & ~parser::E_PARS_EXPECT);
		_responseHandler.getResponse().setFlags(_responseHandler.getResponse().getFlags() & ~E_RESP_HEADERS_SENT);
		return ;
	}

	if (_status == E_CLI_ERR_PARSING)
		_requestHandler.clearBufferRequest();
	_responseHandler.clearBufferResponse();

	if (_responseHandler.getResponse().shouldCloseConnection())
		setStatus(client::E_CLI_DISCONNECTED);
	else
		this->setStatus(E_CLI_REQUEST);

	const config::LocationConfig &locationConfig = _requestHandler.getRequest().getLocationConfig();
	_effectiveKeepaliveTimeout = locationConfig.getKeepAliveTimeout();

	_responseHandler.getResponse().setStatusCode(status::StatusCode());
	_requestHandler.getRequest().getHeaders().clear();
	_responseHandler.getResponse().getHeaders().clear();
	_requestHandler.getParser().setFlags(0);
	_requestHandler.getRequest().setFlags(0);
	_responseHandler.getResponse().setFlags(0);
	_executionHandler.getCgi().reset(_ioMultiplexer);
	_executionHandler.setFlags(0);
	this->setHTTPError(HTTPError());
}

} // !client
} // !webserv
