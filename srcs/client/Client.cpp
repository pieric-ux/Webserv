/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Client.cpp
 * @brief Implements the Client class, which owns a connected client's TCP
 *        socket and drives the per-request HTTP lifecycle: receiving data,
 *        parsing headers and body, executing the request (including CGI I/O),
 *        building responses, sending them, and resetting state for keep-alive.
 */

#include "webserv/parser/Parser.hpp"
#include <webserv/client/Client.hpp>

namespace webserv
{
namespace client
{

/**
 * @brief Constructs a Client with default state and no associated connection,
 *        initializing its handlers from the given I/O multiplexer and acquiring
 *        the client logger.
 *
 * @param ioMultiplexer Shared event-I/O multiplexer used to register and drive
 *        the client's execution (notably CGI) file descriptors.
 */
Client::Client(const t_ioMultiplexer &ioMultiplexer)
	:	_id(-1),
		_socket(),
		_sockaddr_storage(),
		_status(E_CLI_REQUEST),
		_serverConfig(),
		_executionHandler(ioMultiplexer, _sockaddr_storage),
		_requestHandler(_serverConfig),
		_responseHandler(),
		_HTTPError(),
		_lastActivityTime(0),
		_effectiveKeepaliveTimeout(0)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.client");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "Client instance created with default constructor");
}

/**
 * @brief Constructs a Client bound to an accepted connection, seeding its socket
 *        and peer address, selecting server configuration, and initializing the
 *        activity timestamp and effective keep-alive timeout.
 *
 * @param client Accepted socket/address pair: its TCP socket and the peer's
 *        sockaddr_storage.
 * @param serverConfig Server configuration governing this client's request
 *        handling and keep-alive timeout.
 * @param ioMultiplexer Shared event-I/O multiplexer used to register and drive
 *        the client's execution (notably CGI) file descriptors.
 */
Client::Client(const t_SocketPairClient &client, const config::ServerConfig &serverConfig, const t_ioMultiplexer &ioMultiplexer)
	:	_id(-1),
		_socket(client.first),
		_sockaddr_storage(client.second),
		_status(E_CLI_REQUEST),
		_serverConfig(serverConfig),
		_executionHandler(ioMultiplexer, _sockaddr_storage),
		_requestHandler(_serverConfig),
		_responseHandler(),
		_HTTPError(),
		_lastActivityTime(std::time(NULL)),
		_effectiveKeepaliveTimeout(static_cast<std::time_t>(serverConfig.getKeepAliveTimeout()))
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.client.client");
	_logger->setLevel(log42::logRecord::DEBUG);
	DEBUG(_logger, "Client(socket) ctor: this=" + common::core::utils::toString(reinterpret_cast<long>(this))
		+ " socket_fd=" + common::core::utils::toString(_socket.getFd()));
}

/**
 * @brief Destroys the Client; owned resources are released by their RAII members.
 */
Client::~Client() {}

/**
 * @brief Copy-constructs a Client, duplicating its socket, address, status,
 *        handlers, error state and timing, while rebinding the request handler
 *        to this instance's own server configuration.
 *
 * @param rhs Client to copy from.
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
		_effectiveKeepaliveTimeout(rhs._effectiveKeepaliveTimeout)
{
	_requestHandler = rhs._requestHandler;
	DEBUG(_logger, "Client copy ctor: rhs=" + common::core::utils::toString(reinterpret_cast<long>(&rhs))
		+ " rhs.socket_fd=" + common::core::utils::toString(rhs._socket.getFd())
		+ " this=" + common::core::utils::toString(reinterpret_cast<long>(this))
		+ " this.socket_fd=" + common::core::utils::toString(_socket.getFd()));
}

/**
 * @brief Copy-assigns this Client from another, replacing its socket, address,
 *        status, handlers, server configuration, error state and timing
 *        (self-assignment is a no-op).
 *
 * @param rhs Client to copy from.
 * @return Reference to this Client.
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
 * @brief Returns the shared logger for the client module.
 *
 * @return Logger registered under "webserv.client.client".
 */
t_Logger	Client::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.client.client");
}

/**
 * @brief Returns the client's TCP socket.
 *
 * @return Const reference to the underlying connected socket.
 */
const common::core::net::TcpClient &Client::getSocket() const
{
	return _socket;
}

/**
 * @brief Sets the client's TCP socket.
 *
 * @param socket Connected socket to associate with this client.
 */
void Client::setSocket(const common::core::net::TcpClient &socket)
{
	_socket = socket;
}

/**
 * @brief Returns the peer's socket address.
 *
 * @return Copy of the client's sockaddr_storage.
 */
sockaddr_storage Client::getSockaddrStorage() const
{
	return _sockaddr_storage;
}

/**
 * @brief Sets the peer's socket address.
 *
 * @param sockaddr_storage Client address to store.
 */
void Client::setSockaddrStorage(const sockaddr_storage &sockaddr_storage)
{
	_sockaddr_storage = sockaddr_storage;
}

/**
 * @brief Returns the client's current lifecycle status.
 *
 * @return Current status (request, parsing error, or disconnected).
 */
e_ClientStatus Client::getStatus() const
{
	return _status;
}

/**
 * @brief Sets the client's lifecycle status.
 *
 * @param status New status to assign.
 */
void Client::setStatus(const e_ClientStatus status)
{
	_status = status;
}

/**
 * @brief Returns the client's execution handler.
 *
 * @return Reference to the handler performing filesystem and CGI work.
 */
handler::ExecutionHandler &Client::getExecutionHandler()
{
	return _executionHandler;
}

/**
 * @brief Returns the client's request handler.
 *
 * @return Reference to the handler buffering and parsing the request.
 */
handler::RequestHandler &Client::getRequestHandler()
{
	return _requestHandler;
}

/**
 * @brief Returns the client's response handler.
 *
 * @return Reference to the handler building and buffering the response.
 */
handler::ResponseHandler &Client::getResponseHandler()
{
	return _responseHandler;
}

/**
 * @brief Returns the server configuration governing this client.
 *
 * @return Reference to the client's server configuration.
 */
config::ServerConfig &Client::getServerConfig()
{
	return _serverConfig;
}

/**
 * @brief Returns the currently stored HTTP error.
 *
 * @return Reference to the last HTTPError recorded for this client.
 */
HTTPError &Client::getHTTPError()
{
	return _HTTPError;
}

/**
 * @brief Stores an HTTP error to drive a later error response.
 *
 * @param error HTTPError describing the status code and target.
 */
void Client::setHTTPError(const HTTPError &error)
{
	_HTTPError = error;
}

/**
 * @brief Returns the timestamp of the client's last activity.
 *
 * @return Last-activity time as a std::time_t, used for timeout tracking.
 */
std::time_t Client::getLastActivityTime() const
{
	return _lastActivityTime;
}

/**
 * @brief Sets the timestamp of the client's last activity.
 *
 * @param time New last-activity time, typically the current time.
 */
void Client::setLastActivityTime(const std::time_t time)
{
	_lastActivityTime = time;
}

/**
 * @brief Returns the effective keep-alive timeout for this client.
 *
 * @return Keep-alive timeout in seconds applied to the current connection.
 */
std::time_t Client::getEffectiveKeepaliveTimeout() const
{
	return _effectiveKeepaliveTimeout;
}

/**
 * @brief Sets the effective keep-alive timeout for this client.
 *
 * @param timeout Keep-alive timeout in seconds to apply.
 */
void Client::setEffectiveKeepaliveTimeout(const std::time_t timeout)
{
	_effectiveKeepaliveTimeout = timeout;
}

/**
 * @brief Reads available bytes from the socket into the request handler's buffer.
 *
 * On a recv error or an orderly peer shutdown (zero bytes read) the client is
 * marked disconnected; otherwise the received bytes are appended to the pending
 * request buffer.
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
 * @brief Sends as much of the pending response buffer as the socket accepts.
 *
 * Returns immediately when there is nothing to send; on a send error the client
 * is marked disconnected. The bytes actually written are removed from the front
 * of the response buffer, and the response's headers-sent flag is set on the
 * first successful send.
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
 * @brief Advances the HTTP request through one processing step on buffered data.
 *
 * Parses headers if not yet validated, parses the body for methods that carry one
 * (unless an Expect: 100-continue is pending), runs non-CGI execution once headers
 * are ready, and builds the response headers when appropriate (including an interim
 * 100 Continue, or pushing a completed CGI body after its headers). Any HTTPError
 * raised along the way is stored and switches the client to the parsing-error state.
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
			!(_executionHandler.getFlags() & handler::E_EXEC_COMPLETE)) // skipping execution once complete does not affect 100-continue: the E_PARS_EXPECT guard above already defers execution until the interim 100 Continue has been sent and the flag cleared
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
 * @brief Builds the HTTP error response from the stored HTTPError.
 *
 * Does nothing if the response headers have already been sent; otherwise builds
 * the error response and marks execution complete so no further work is attempted.
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
 * @brief Determines whether the current request resolves to a CGI route.
 *
 * @return true if the request's headers are validated, its matched location has
 *         CGI enabled, and the resolved target's file extension is among that
 *         location's configured CGI extensions; false otherwise.
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
 * @brief Advances CGI execution I/O for the current request when it is a CGI route.
 *
 * Returns immediately for non-CGI routes; otherwise steps CGI execution until it
 * completes, and once the CGI is complete and the response headers have been sent,
 * pushes the CGI body into the response buffer. An HTTPError is stored and moves
 * the client to the parsing-error state.
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
				_requestHandler.getRequest().getLocationConfig());
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
 * @brief Resets per-request state in preparation for the next request on a
 *        keep-alive connection.
 *
 * For a pending Expect: 100-continue, only clears the expect and headers-sent
 * flags so the actual request can proceed. Otherwise clears the request and
 * response buffers (the request buffer only on a parsing error), decides whether
 * to close or keep the connection alive, refreshes the effective keep-alive
 * timeout from the matched location, and resets status code, headers, parser,
 * request, response, execution and CGI state along with the stored HTTP error.
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
	_executionHandler.getCgi().reset();
	_executionHandler.setFlags(0);
	this->setHTTPError(HTTPError());
}

} // !client
} // !webserv
