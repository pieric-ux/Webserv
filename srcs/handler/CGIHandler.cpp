/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file CGIHandler.cpp
 * @brief Implements CGIHandler: forking a CGI interpreter, building its
 *        environment, streaming the request body to its stdin, reading its
 *        stdout, reaping the child, and parsing its output into an HTTP
 *        response.
 */

#include <webserv/handler/CGIHandler.hpp>
#include <webserv/handler/ExecutionHandler.hpp>
#include <webserv/handler/RequestHandler.hpp>
#include <webserv/handler/ResponseHandler.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/config/DefaultConfig.hpp>
#include <webserv/config/method.hpp>
#include <webserv/status/StatusCodeRegistry.hpp>

#include <abnf/Abnf.hpp>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <utility>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

namespace webserv
{
namespace handler
{

/**
 * @brief Constructs a CGIHandler bound to an I/O multiplexer and client
 *        address, initializing all process state to its unspawned defaults and
 *        acquiring the handler's logger.
 */
CGIHandler::CGIHandler(const t_ioMultiplexer &ioMultiplexer, sockaddr_storage clientAddr)
	:	_ioMultiplexer(ioMultiplexer),
		_clientAddr(clientAddr),
		_pid(-1),
		_stdinFd(-1),
		_stdoutFd(-1),
		_cgiStartTime(0),
		_envBuilder(),
		_envp(),
		_cgiBuffer(),
		_cgiResponseBody(),
		_bodySent(0),
		_exitStatus(0),
		_spawned(false),
		_bodySentDone(false),
		_eof(false),
		_reaped(false),
		_parsed(false),
		_pushed(false)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.handler.cgihandler");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "CGIHandler instance created");
}

/**
 * @brief Destroys the handler, killing and reaping the child process if it was
 *        spawned but not yet reaped to avoid leaving a zombie.
 */
CGIHandler::~CGIHandler()
{
	DEBUG(_logger, "CGIHandler instance destroyed pid=" + common::core::utils::toString(_pid)
		+ " spawned=" + common::core::utils::toString(_spawned)
		+ " reaped=" + common::core::utils::toString(_reaped));
	if (_pid > 0 && !_reaped)
		killAndReap();
}

/**
 * @brief Returns the shared logger used by this handler.
 *
 * @return The "webserv.handler.cgihandler" logger from the log42 manager.
 */
t_Logger CGIHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.cgihandler");
}

/**
 * @brief Resolves the interpreter for the script's extension, validates that
 *        the script is executable, builds the CGI environment, creates the
 *        stdin/stdout pipes, forks and execs the interpreter, and registers the
 *        pipe ends with the I/O multiplexer. Throws HTTPError on failure (500
 *        for missing interpreter or syscall errors, 404/403 for access errors).
 *
 * @param request The client request whose absolute path, query and headers
 *        drive the CGI invocation and environment.
 * @param locationConfig The matched location configuration, queried for the
 *        extension-to-interpreter mapping.
 */
void CGIHandler::spawn(const client::Request &request,
		const config::LocationConfig &locationConfig)
{
	DEBUG(_logger, "spawn: enter spawned=" + common::core::utils::toString(_spawned));
	if (_spawned)
	{
		DEBUG(_logger, "spawn: already spawned, skip");
		return ;
	}

	const std::string scriptPath = request.getAbsolutePath();
	DEBUG(_logger, "spawn: scriptPath=" + scriptPath);

	std::string ext;
	{
		std::string::size_type dot = scriptPath.rfind('.');
		if (dot != std::string::npos)
			ext = scriptPath.substr(dot);
	}
	DEBUG(_logger, "spawn: extension=\"" + ext + "\"");

	const t_CgiExtensions &exts = locationConfig.getCgiExtensions();
	t_CgiExtensions::const_iterator extIt = exts.find(ext);
	if (extIt == exts.end())
	{
		ERROR(_logger, "spawn: no interpreter for extension \"" + ext + "\"");
		throw client::HTTPError(500);
	}
	const std::string interpreter = extIt->second;
	DEBUG(_logger, "spawn: interpreter resolved -> " + interpreter);

	errno = 0;
	if (::access(scriptPath.c_str(), X_OK) == -1)
	{
		if (errno == ENOENT)
		{
			INFO(_logger, "spawn: 404 script not found: " + scriptPath);
			throw client::HTTPError(404);
		}
		if (errno == EACCES)
		{
			INFO(_logger, "spawn: 403 script not executable: " + scriptPath);
			throw client::HTTPError(403);
		}
		throw client::HTTPError(500);
	}
	DEBUG(_logger, "spawn: X_OK passed");

	buildEnv(request,interpreter, scriptPath);
	finalizeEnvp();

	int rawStdin[2] = { -1, -1 };
	if (::pipe(rawStdin) == -1)
	{
		ERROR(_logger, "spawn: pipe(stdin) failed");
		throw client::HTTPError(500);
	}
	common::core::raii::UniqueFd stdinR(rawStdin[0]);
	common::core::raii::UniqueFd stdinW(rawStdin[1]);
	DEBUG(_logger, "spawn: stdin pipe r=" + common::core::utils::toString(rawStdin[0])
		+ " w=" + common::core::utils::toString(rawStdin[1]));

	int rawStdout[2] = { -1, -1 };
	if (::pipe(rawStdout) == -1)
	{
		ERROR(_logger, "spawn: pipe(stdout) failed");
		throw client::HTTPError(500);
	}
	common::core::raii::UniqueFd stdoutR(rawStdout[0]);
	common::core::raii::UniqueFd stdoutW(rawStdout[1]);
	DEBUG(_logger, "spawn: stdout pipe r=" + common::core::utils::toString(rawStdout[0])
		+ " w=" + common::core::utils::toString(rawStdout[1]));

	std::string scriptDir;
	{
		std::string::size_type slash = scriptPath.rfind('/');
		scriptDir = (slash == std::string::npos) ? "." : scriptPath.substr(0, slash);
		if (scriptDir.empty())
			scriptDir = "/";
	}
	DEBUG(_logger, "spawn: scriptDir=" + scriptDir);

	DEBUG(_logger, "spawn: forking...");
	_pid = ::fork();
	if (_pid == -1)
	{
		ERROR(_logger, "spawn: fork() failed");
		throw client::HTTPError(500);
	}

	if (_pid == 0)
	{
		if (::dup2(stdinR.get(), STDIN_FILENO) == -1)
			::_exit(127);
		if (::dup2(stdoutW.get(), STDOUT_FILENO) == -1)
			::_exit(127);
		stdinR.reset();
		stdinW.reset();
		stdoutR.reset();
		stdoutW.reset();

		::chdir(scriptDir.c_str());

		char *argv[3];
		argv[0] = const_cast<char *>(interpreter.c_str());
		argv[1] = const_cast<char *>(scriptPath.c_str());
		argv[2] = NULL;
		::execve(interpreter.c_str(), argv, _envp.get());
		::_exit(127);
	}

	DEBUG(_logger, "spawn: parent — child pid=" + common::core::utils::toString(_pid));

	_stdinFd.reset(stdinW.release());
	_stdoutFd.reset(stdoutR.release());
	DEBUG(_logger, "spawn: ownership transferred stdin=" + common::core::utils::toString(_stdinFd.get())
		+ " stdout=" + common::core::utils::toString(_stdoutFd.get()));

	try {
		setNonblock(_stdinFd.get());
		setNonblock(_stdoutFd.get());
	}
	catch (...) {
		ERROR(_logger, "spawn: setNonblock failed, killing child");
		killAndReap();
		throw;
	}
	DEBUG(_logger, "spawn: O_NONBLOCK set on both pipes");

	const t_Headers &headers = request.getHeaders();
	t_Headers::const_iterator clIt = headers.find("content-length");
	bool hasBody = (clIt != headers.end() && !clIt->second.empty()
			&& std::strtoul(clIt->second.front().getValue().c_str(), NULL, 10) > 0);
	DEBUG(_logger, "spawn: hasBody=" + common::core::utils::toString(hasBody));

	if (hasBody)
	{
		_ioMultiplexer->add(_stdinFd.get(), common::core::io::IEventIO::E_OUT);
		DEBUG(_logger, "spawn: registered stdin fd=" + common::core::utils::toString(_stdinFd.get()) + " E_OUT");
	}
	else
	{
		DEBUG(_logger, "spawn: no body, closing stdin to signal EOF to child");
		_stdinFd.reset();
		_bodySentDone = true;
	}

	_ioMultiplexer->add(_stdoutFd.get(), common::core::io::IEventIO::E_IN);
	DEBUG(_logger, "spawn: registered stdout fd=" + common::core::utils::toString(_stdoutFd.get()) + " E_IN");

	_cgiStartTime = std::time(NULL);
	_spawned = true;
	INFO(_logger, "spawn: pid=" + common::core::utils::toString(_pid)
		+ " script=" + scriptPath
		+ " interpreter=" + interpreter);
}

/**
 * @brief Advances one I/O step of the CGI exchange: checks for timeout, writes
 *        a chunk of the request body to the child's stdin and reads a chunk
 *        from its stdout when the respective pipe is ready, and reaps the child
 *        once end-of-file is reached. No-op if not spawned or execution is
 *        already complete.
 *
 * @param requestHandler The request handler supplying the body bytes streamed
 *        to the CGI's stdin.
 * @param execFlags Execution flags; returns immediately if E_EXEC_COMPLETE is
 *        set.
 */
void CGIHandler::driveIO(handler::RequestHandler &requestHandler, int execFlags)
{
	if (!_spawned || (execFlags & E_EXEC_COMPLETE))
		return ;

	DEBUG(_logger, "driveIO: enter spawned=" + common::core::utils::toString(_spawned)
		+ " stdinValid=" + common::core::utils::toString(_stdinFd.valid())
		+ " stdoutValid=" + common::core::utils::toString(_stdoutFd.valid())
		+ " eof=" + common::core::utils::toString(_eof)
		+ " reaped=" + common::core::utils::toString(_reaped));

	checkTimeout();

	if (_stdinFd.valid())
	{
		common::core::io::IEventIO::e_Event ev = _ioMultiplexer->getEvents(_stdinFd.get());
		if (ev & common::core::io::IEventIO::E_OUT)
		{
			DEBUG(_logger, "driveIO: stdin ready -> write");
			writeChunkToCGI(requestHandler);
		}
	}

	if (_stdoutFd.valid())
	{
		common::core::io::IEventIO::e_Event ev = _ioMultiplexer->getEvents(_stdoutFd.get());
		if (ev & common::core::io::IEventIO::E_IN)
		{
			DEBUG(_logger, "driveIO: stdout ready -> read");
			readChunkFromCGI();
		}
	}

	if (_eof && !_reaped)
	{
		DEBUG(_logger, "driveIO: eof reached -> tryReap");
		tryReap();
	}
}

/**
 * @brief Parses the buffered CGI output, splitting it at the header/body
 *        separator, validating and applying each header line (handling Status,
 *        Location and Content-Type), and defaulting Content-Length. The body is
 *        retained for later push. Throws HTTPError(502) on malformed output or
 *        when none of Content-Type, Location or Status is present.
 *
 * @param response The response object the parsed status code and headers are
 *        written into.
 */
void CGIHandler::parse(client::Response &response)
{
	if (_parsed)
	{
		DEBUG(_logger, "parse: already parsed, skip");
		return ;
	}

	DEBUG(_logger, "parse: enter buffer=" + common::core::utils::toString(_cgiBuffer.size()) + " bytes");

	std::pair<std::size_t, std::size_t> sep = findHeaderEnd(_cgiBuffer);
	if (sep.first == std::string::npos)
	{
		ERROR(_logger, "parse: no CRLFCRLF/LFLF separator found, malformed CGI output");
		throw client::HTTPError(502);
	}
	DEBUG(_logger, "parse: header/body separator at offset=" + common::core::utils::toString(sep.first)
		+ " sepLen=" + common::core::utils::toString(sep.second));

	std::string headerBlock(_cgiBuffer.begin(), _cgiBuffer.begin() + sep.first);
	_cgiResponseBody.assign(_cgiBuffer.begin() + sep.first + sep.second, _cgiBuffer.end());
	_cgiBuffer.clear();
	DEBUG(_logger, "parse: headerBlock=" + common::core::utils::toString(headerBlock.size())
		+ " body=" + common::core::utils::toString(_cgiResponseBody.size()));

	bool hasContentType = false;
	bool hasLocation = false;
	bool hasStatus = false;

	std::size_t pos = 0;
	while (pos < headerBlock.size())
	{
		std::size_t lineEnd = headerBlock.find('\n', pos);
		if (lineEnd == std::string::npos)
			lineEnd = headerBlock.size();
		std::string line = headerBlock.substr(pos, lineEnd - pos);
		pos = lineEnd + 1;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		if (!abnf::Abnf::getInstance().match("field-line", "HTTP", line))
		{
			ERROR(_logger, "parse: invalid field-line per RFC 9110: \"" + line + "\"");
			throw client::HTTPError(502);
		}

		std::size_t colon = line.find(':');
		if (colon == std::string::npos)
		{
			ERROR(_logger, "parse: header line without colon: \"" + line + "\"");
			throw client::HTTPError(502);
		}
		std::string name = common::core::utils::trim(line.substr(0, colon));
		std::string value = common::core::utils::trim(line.substr(colon + 1));
		if (name.empty())
		{
			ERROR(_logger, "parse: empty header name");
			throw client::HTTPError(502);
		}
		std::string lower = common::core::utils::toLower(name);

		if (lower == "status")
		{
			std::size_t sp = value.find(' ');
			std::string codeStr = (sp == std::string::npos) ? value : value.substr(0, sp);
			char *endp = NULL;
			long code = std::strtol(codeStr.c_str(), &endp, 10);
			if (codeStr.empty() || endp == codeStr.c_str() || code < 100 || code > 599)
			{
				ERROR(_logger, "parse: invalid Status header value: \"" + value + "\"");
				throw client::HTTPError(502);
			}
			response.setStatusCode(
				status::StatusCodeRegistry::getInstance().getStatusCode(
					static_cast<unsigned short>(code)));
			hasStatus = true;
			DEBUG(_logger, "parse: Status -> " + common::core::utils::toString(code));
			continue;
		}
		if (lower == "location")
			hasLocation = true;
		if (lower == "content-type")
			hasContentType = true;

		response.addHeader(name, value);
		DEBUG(_logger, "parse: header " + name + ": " + value);
	}

	if (!hasContentType && !hasLocation && !hasStatus)
	{
		ERROR(_logger, "parse: missing Content-Type / Location / Status header");
		throw client::HTTPError(502);
	}

	if (response.findHeader("content-length", "").getName().empty())
	{
		response.addHeader("Content-Length",
			common::core::utils::toString(_cgiResponseBody.size()));
		DEBUG(_logger, "parse: Content-Length defaulted to " + common::core::utils::toString(_cgiResponseBody.size()));
	}

	_parsed = true;
	INFO(_logger, "parse: done body=" + common::core::utils::toString(_cgiResponseBody.size()) + " bytes");
}

/**
 * @brief Appends the parsed CGI response body to the response handler's output
 *        buffer once, then clears the body. Idempotent: subsequent calls are
 *        no-ops.
 *
 * @param responseHandler The response handler whose output buffer the CGI body
 *        is appended to.
 */
void CGIHandler::pushBody(handler::ResponseHandler &responseHandler)
{
	if (_pushed)
	{
		DEBUG(_logger, "pushBody: already pushed, skip");
		return ;
	}
	if (!_cgiResponseBody.empty())
	{
		DEBUG(_logger, "pushBody: appending " + common::core::utils::toString(_cgiResponseBody.size()) + " bytes to response");
		responseHandler.appendToBufferResponse(_cgiResponseBody);
	}
	_cgiResponseBody.clear();
	_pushed = true;
}

/**
 * @brief Releases all CGI resources: unregisters and closes the pipe fds, kills
 *        and reaps any surviving child, frees the environment, clears the
 *        buffers, and restores every state flag to its unspawned default so the
 *        handler can be reused.
 */
void CGIHandler::reset()
{
	if (_stdinFd.valid())
	{
		try { _ioMultiplexer->remove(_stdinFd.get()); } catch (...) {}
		_stdinFd.reset();
	}
	if (_stdoutFd.valid())
	{
		try { _ioMultiplexer->remove(_stdoutFd.get()); } catch (...) {}
		_stdoutFd.reset();
	}
	if (_pid > 0 && !_reaped)
		killAndReap();

	_envp.reset();
	_envBuilder.clear();
	_cgiBuffer.clear();
	_cgiResponseBody.clear();
	_pid = -1;
	_cgiStartTime = 0;
	_bodySent = 0;
	_exitStatus = 0;
	_spawned = false;
	_bodySentDone = false;
	_eof = false;
	_reaped = false;
	_parsed = false;
	_pushed = false;
	DEBUG(_logger, "reset: state cleared");
}

/**
 * @brief Returns the I/O multiplexer this handler registers its pipes with.
 *
 * @return The shared event multiplexer held by the handler.
 */
t_ioMultiplexer CGIHandler::getIoMultiplexer() const
{
	return _ioMultiplexer;
}

/**
 * @brief Returns the client's address used to populate the REMOTE_* CGI
 *        variables.
 *
 * @return The stored client socket address.
 */
sockaddr_storage	CGIHandler::getClientAddr() const
{
	return _clientAddr;
}

/**
 * @brief Reports whether either CGI pipe fd is still open.
 *
 * @return True if the stdin or stdout pipe fd is still valid.
 */
bool CGIHandler::hasFds() const
{
	return _stdinFd.valid() || _stdoutFd.valid();
}

/**
 * @brief Returns the write end of the pipe connected to the CGI process's
 *        stdin.
 *
 * @return The stdin pipe fd, or -1 if it has been closed.
 */
int CGIHandler::getStdinFd() const
{
	return _stdinFd.get();
}

/**
 * @brief Returns the read end of the pipe connected to the CGI process's
 *        stdout.
 *
 * @return The stdout pipe fd, or -1 if it has been closed.
 */
int CGIHandler::getStdoutFd() const
{
	return _stdoutFd.get();
}

/**
 * @brief Reports whether the CGI process has been forked.
 *
 * @return True once spawn() has successfully started the child.
 */
bool CGIHandler::isSpawned() const
{
	return _spawned;
}

/**
 * @brief Reports whether the CGI child has been waited on.
 *
 * @return True once the child process has been reaped.
 */
bool CGIHandler::isReaped() const
{
	return _reaped;
}

/**
 * @brief Reports whether the CGI output has been parsed into the response.
 *
 * @return True once parse() has processed the buffered CGI output.
 */
bool CGIHandler::isParsed() const
{
	return _parsed;
}

/**
 * @brief Reports whether the CGI body has been pushed to the response handler.
 *
 * @return True once pushBody() has appended the body to the response buffer.
 */
bool CGIHandler::isPushed() const
{
	return _pushed;
}

/**
 * @brief Converts an HTTP header name to its CGI environment variable form by
 *        prefixing "HTTP_", upper-casing the name, and replacing hyphens with
 *        underscores.
 *
 * @param name The HTTP header field name to convert.
 * @return The corresponding HTTP_* CGI variable name.
 */
std::string CGIHandler::headerToCgiName(const std::string &name)
{
	std::string out = "HTTP_";
	for (std::size_t i = 0; i < name.size(); ++i)
	{
		char c = name[i];
		out += (c == '-') ? '_' : static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	}
	return out;
}

/**
 * @brief Sets the O_NONBLOCK flag on a file descriptor, throwing HTTPError(500)
 *        if the fcntl call fails.
 *
 * @param fd The file descriptor to switch to non-blocking mode.
 */
void CGIHandler::setNonblock(int fd)
{
	if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw client::HTTPError(500);
}

/**
 * @brief Locates the header/body separator in raw CGI output, accepting either
 *        a CRLFCRLF or an LFLF sequence.
 *
 * @param v The raw CGI output bytes to scan.
 * @return A pair of (offset of the separator, separator length); the offset is
 *         std::string::npos with length 0 when no separator is found.
 */
std::pair<std::size_t, std::size_t> CGIHandler::findHeaderEnd(const t_raw &v)
{
	for (std::size_t i = 0; i + 1 < v.size(); ++i)
	{
		if (i + 3 < v.size() && v[i] == '\r' && v[i+1] == '\n'
				&& v[i+2] == '\r' && v[i+3] == '\n')
			return std::make_pair(i, static_cast<std::size_t>(4));
		if (v[i] == '\n' && v[i+1] == '\n')
			return std::make_pair(i, static_cast<std::size_t>(2));
	}
	return std::make_pair(std::string::npos, static_cast<std::size_t>(0));
}

/**
 * @brief Populates the environment builder with the CGI/1.1 meta-variables
 *        (CONTENT_LENGTH, CONTENT_TYPE, GATEWAY_INTERFACE, QUERY_STRING,
 *        REQUEST_METHOD, SCRIPT_*, SERVER_*, REMOTE_*, etc.) derived from the
 *        request and client address, then adds every other request header as an
 *        HTTP_* variable with multiple values comma-joined.
 *
 * @param request The client request supplying the method, query, target,
 *        path and headers.
 * @param interpreter The resolved interpreter path (logged for diagnostics).
 * @param scriptPath The absolute script path exported as SCRIPT_FILENAME.
 */
void CGIHandler::buildEnv(const client::Request &request,
		const std::string &interpreter,
		const std::string &scriptPath)
{
	DEBUG(_logger, "buildEnv: enter method=" + config::methodToStr(request.getMethod())
		+ " script=" + scriptPath
		+ " interpreter=" + interpreter
		+ " query=\"" + request.getQuery() + "\""
		+ " headers=" + common::core::utils::toString(request.getHeaders().size()));

	const t_Headers &headers = request.getHeaders();

	t_Headers::const_iterator clIt = headers.find("content-length");
	addEnv("CONTENT_LENGTH",
		(clIt != headers.end() && !clIt->second.empty())
		? clIt->second.front().getValue() : "0");

	t_Headers::const_iterator ctIt = headers.find("content-type");
	addEnv("CONTENT_TYPE",
		(ctIt != headers.end() && !ctIt->second.empty())
		? ctIt->second.front().getValue() : "");

	addEnv("GATEWAY_INTERFACE", "CGI/1.1");
	addEnv("QUERY_STRING", request.getQuery());
	addEnv("REQUEST_METHOD", config::methodToStr(request.getMethod()));
	addEnv("REQUEST_URI", request.getRequestTarget());
	addEnv("SCRIPT_NAME", request.getPath());
	addEnv("SCRIPT_FILENAME", scriptPath);
	addEnv("PATH_INFO", "");
	addEnv("PATH_TRANSLATED", "");
	addEnv("AUTH_TYPE", "");
	addEnv("SERVER_NAME", "webserv");
	addEnv("SERVER_PROTOCOL", "HTTP/1.1");
	addEnv("SERVER_SOFTWARE", "webserv/1.0");
	addEnv("SERVER_PORT", "8080");

	std::string remoteAddr;
	std::string remotePort;
	try {
		t_AddrPortPair addr = common::core::net::getNameInfo(_clientAddr);
		remoteAddr = addr.first;
		remotePort = addr.second;
	}
	catch (const std::exception &e) {
		WARNING(_logger, "buildEnv: getNameInfo failed: " + std::string(e.what()));
	}
	addEnv("REMOTE_ADDR", remoteAddr);
	addEnv("REMOTE_HOST", remoteAddr);
	addEnv("REMOTE_PORT", remotePort);
	addEnv("REMOTE_IDENT", "");
	addEnv("REMOTE_USER", "");

	for (t_Headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == "content-length" || it->first == "content-type")
			continue;
		if (it->second.empty())
			continue;
		std::string joined;
		for (std::list<HTTPheaders::HTTPHeader>::const_iterator vit = it->second.begin();
				vit != it->second.end(); ++vit)
		{
			if (!joined.empty())
				joined += ", ";
			joined += vit->getValue();
		}
		addEnv(headerToCgiName(it->first), joined);
	}
	DEBUG(_logger, "buildEnv: done, total=" + common::core::utils::toString(_envBuilder.size()));
}

/**
 * @brief Appends a "key=value" entry to the environment builder.
 *
 * @param key The environment variable name.
 * @param value The environment variable value.
 */
void CGIHandler::addEnv(const std::string &key, const std::string &value)
{
	DEBUG(_logger, "addEnv " + key + "=" + value);
	_envBuilder.push_back(key + "=" + value);
}

/**
 * @brief Builds the NULL-terminated char* array passed to execve from the
 *        accumulated environment strings, with each pointer referencing the
 *        builder's stored entries.
 *
 * @return The NULL-terminated environment pointer array (also stored in _envp).
 */
char **CGIHandler::finalizeEnvp()
{
	std::size_t n = _envBuilder.size();
	_envp.reset(new char *[n + 1]);
	for (std::size_t i = 0; i < n; ++i)
		_envp[i] = const_cast<char *>(_envBuilder[i].c_str());
	_envp[n] = NULL;
	DEBUG(_logger, "finalizeEnvp size=" + common::core::utils::toString(n));
	return _envp.get();
}

/**
 * @brief Forcibly terminates the child with SIGKILL and blocks in waitpid to
 *        reap it, recording the exit status and marking the handler as reaped.
 *        No-op if there is no live, unreaped child.
 */
void CGIHandler::killAndReap()
{
	if (_pid <= 0 || _reaped)
		return ;
	::kill(_pid, SIGKILL);
	int status = 0;
	::waitpid(_pid, &status, 0);
	_pid = -1;
	_reaped = true;
	_exitStatus = status;
	DEBUG(_logger, "killAndReap: child reaped status=" + common::core::utils::toString(status));
}

/**
 * @brief Non-blocking reap of the child via waitpid(WNOHANG). Returns silently
 *        if the child is still running; on exit it records the status and
 *        throws HTTPError(502) when the child was killed by a signal or exited
 *        non-zero without producing any output.
 */
void CGIHandler::tryReap()
{
	if (_pid <= 0 || _reaped)
		return ;
	DEBUG(_logger, "tryReap: waitpid(WNOHANG) on pid=" + common::core::utils::toString(_pid));
	int status = 0;
	pid_t rc = ::waitpid(_pid, &status, WNOHANG);
	if (rc == 0)
	{
		DEBUG(_logger, "tryReap: child still alive");
		return ;
	}
	if (rc == _pid)
	{
		_reaped = true;
		_exitStatus = status;
		_pid = -1;
		DEBUG(_logger, "tryReap: child reaped status=" + common::core::utils::toString(status));
		if (WIFSIGNALED(status))
		{
			ERROR(_logger, "tryReap: child died by signal " + common::core::utils::toString(WTERMSIG(status)));
			throw client::HTTPError(502);
		}
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0 && _cgiBuffer.empty())
		{
			ERROR(_logger, "tryReap: child exited " + common::core::utils::toString(WEXITSTATUS(status)) + " with no output");
			throw client::HTTPError(502);
		}
		return ;
	}
	if (rc == -1 && errno == ECHILD)
		_reaped = true;
}

/**
 * @brief Enforces the CGI execution deadline: if the spawned, unreaped child
 *        has run longer than CGI_TIMEOUT_S, kills and reaps it and throws
 *        HTTPError(504).
 */
void CGIHandler::checkTimeout()
{
	if (_reaped || !_spawned)
		return ;
	std::time_t now = std::time(NULL);
	if (now - _cgiStartTime > static_cast<std::time_t>(CGI_TIMEOUT_S))
	{
		WARNING(_logger, "checkTimeout: 504 elapsed=" + common::core::utils::toString(now - _cgiStartTime) + "s");
		killAndReap();
		throw client::HTTPError(504);
	}
}

/**
 * @brief Writes one chunk of the request body to the child's stdin, bounded by
 *        the declared Content-Length and the configured buffer size, advancing
 *        the request buffer and the sent-byte counter. Closes stdin once the
 *        full body has been sent (or when no Content-Length is present), and
 *        throws HTTPError(502) on write failure.
 *
 * @param requestHandler The request handler providing the body buffer and
 *        Content-Length, whose buffer front is erased as bytes are sent.
 */
void CGIHandler::writeChunkToCGI(handler::RequestHandler &requestHandler)
{
	const t_raw &buf = requestHandler.getBufferRequest();
	const t_Headers &headers = requestHandler.getRequest().getHeaders();
	t_Headers::const_iterator clIt = headers.find("content-length");
	if (clIt == headers.end() || clIt->second.empty())
	{
		DEBUG(_logger, "writeChunkToCGI: no Content-Length, closing stdin");
		_ioMultiplexer->remove(_stdinFd.get());
		_stdinFd.reset();
		_bodySentDone = true;
		return ;
	}
	std::size_t total = std::strtoul(clIt->second.front().getValue().c_str(), NULL, 10);

	if (buf.empty())
	{
		DEBUG(_logger, "writeChunkToCGI: client buffer empty, sent=" + common::core::utils::toString(_bodySent) + "/" + common::core::utils::toString(total));
		if (_bodySent >= total)
		{
			DEBUG(_logger, "writeChunkToCGI: body fully sent, closing stdin");
			_ioMultiplexer->remove(_stdinFd.get());
			_stdinFd.reset();
			_bodySentDone = true;
		}
		return ;
	}

	std::size_t remaining = total - _bodySent;
	std::size_t toWrite = std::min(remaining,
			std::min(static_cast<std::size_t>(config::DefaultConfig::BUFFER_SIZE), buf.size()));

	ssize_t wr = ::write(_stdinFd.get(), &buf[0], toWrite);
	if (wr > 0)
	{
		requestHandler.eraseBufferRequestFront(static_cast<std::size_t>(wr));
		_bodySent += static_cast<std::size_t>(wr);
		DEBUG(_logger, "writeChunkToCGI: " + common::core::utils::toString(wr)
			+ " bytes (" + common::core::utils::toString(_bodySent) + "/"
			+ common::core::utils::toString(total) + ")");
		if (_bodySent >= total)
		{
			_ioMultiplexer->remove(_stdinFd.get());
			_stdinFd.reset();
			_bodySentDone = true;
		}
		return ;
	}
	ERROR(_logger, "writeChunkToCGI: write failed, terminating CGI");
	_ioMultiplexer->remove(_stdinFd.get());
	_stdinFd.reset();
	throw client::HTTPError(502);
}

/**
 * @brief Reads one chunk from the child's stdout into the CGI buffer; sets the
 *        EOF flag on a zero-length read and throws HTTPError(502) on read
 *        failure.
 */
void CGIHandler::readChunkFromCGI()
{
	unsigned char buf[config::DefaultConfig::BUFFER_SIZE];
	ssize_t rd = ::read(_stdoutFd.get(), buf, sizeof(buf));
	if (rd > 0)
	{
		_cgiBuffer.insert(_cgiBuffer.end(), buf, buf + rd);
		DEBUG(_logger, "readChunkFromCGI: " + common::core::utils::toString(rd)
			+ " bytes (total=" + common::core::utils::toString(_cgiBuffer.size()) + ")");
		return ;
	}
	if (rd == 0)
	{
		_eof = true;
		DEBUG(_logger, "readChunkFromCGI: EOF on stdout (total=" + common::core::utils::toString(_cgiBuffer.size()) + ")");
		return ;
	}
	ERROR(_logger, "readChunkFromCGI: read failed");
	throw client::HTTPError(502);
}

} // !handler
} // !webserv
