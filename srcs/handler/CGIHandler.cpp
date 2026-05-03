// TODO: don't forget header

/**
 * @file CGIHandler.cpp
 * @brief [TODO:description]
 */

#include <webserv/handler/CGIHandler.hpp>
#include <webserv/handler/RequestHandler.hpp>
#include <webserv/handler/ResponseHandler.hpp>
#include <webserv/client/Client.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/config/DefaultConfig.hpp>
#include <webserv/config/method.hpp>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <ctime>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

namespace webserv
{
namespace handler
{

namespace
{
	/**
	 * @brief [TODO:description]
	 *
	 * @param name [TODO:parameter]
	 * @return [TODO:return]
	 */
	std::string headerToCgiName(const std::string &name)
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
	 * @brief [TODO:description]
	 *
	 * @param fd [TODO:parameter]
	 */
	void setNonblock(int fd)
	{
		if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
			throw client::HTTPError(500);
	}
}

/**
 * @brief [TODO:description]
 */
CGIHandler::CGIHandler()
	:	_pid(-1),
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
 * @brief [TODO:description]
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
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger CGIHandler::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.handler.cgihandler");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool CGIHandler::hasFds() const
{
	return _stdinFd.valid() || _stdoutFd.valid();
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int CGIHandler::getStdinFd() const
{
	return _stdinFd.get();
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int CGIHandler::getStdoutFd() const
{
	return _stdoutFd.get();
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool CGIHandler::isSpawned() const
{
	return _spawned;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool CGIHandler::isReaped() const
{
	return _reaped;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool CGIHandler::isParsed() const
{
	return _parsed;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool CGIHandler::isPushed() const
{
	return _pushed;
}

/**
 * @brief [TODO:description]
 *
 * @param key [TODO:parameter]
 * @param value [TODO:parameter]
 */
void CGIHandler::addEnv(const std::string &key, const std::string &value)
{
	DEBUG(_logger, "addEnv " + key + "=" + value);
	_envBuilder.push_back(key + "=" + value);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
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
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 * @param client [TODO:parameter]
 * @param interpreter [TODO:parameter]
 * @param scriptPath [TODO:parameter]
 */
void CGIHandler::buildEnv(const client::Request &request,
		const config::LocationConfig &locationConfig,
		const client::Client &client,
		const std::string &interpreter,
		const std::string &scriptPath)
{
	(void)interpreter;
	(void)locationConfig;

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
		t_AddrPortPair addr = common::core::net::getNameInfo(client.getSockaddrStorage());
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
 * @brief [TODO:description]
 *
 * @param request [TODO:parameter]
 * @param locationConfig [TODO:parameter]
 * @param client [TODO:parameter]
 * @param mux [TODO:parameter]
 */
void CGIHandler::spawn(const client::Request &request,
		const config::LocationConfig &locationConfig,
		const client::Client &client,
		t_ioMultiplexer mux)
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
	if (::access(scriptPath.c_str(), F_OK) == -1)
	{
		INFO(_logger, "spawn: 404 script not found: " + scriptPath);
		throw client::HTTPError(404);
	}
	DEBUG(_logger, "spawn: F_OK passed");
	errno = 0;
	if (::access(scriptPath.c_str(), X_OK) == -1)
	{
		if (errno == EACCES)
		{
			INFO(_logger, "spawn: 403 script not executable: " + scriptPath);
			throw client::HTTPError(403);
		}
		throw client::HTTPError(500);
	}
	DEBUG(_logger, "spawn: X_OK passed");

	buildEnv(request, locationConfig, client, interpreter, scriptPath);
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
		mux->add(_stdinFd.get(), common::core::io::IEventIO::E_OUT);
		DEBUG(_logger, "spawn: registered stdin fd=" + common::core::utils::toString(_stdinFd.get()) + " E_OUT");
	}
	else
	{
		DEBUG(_logger, "spawn: no body, closing stdin to signal EOF to child");
		_stdinFd.reset();
		_bodySentDone = true;
	}

	mux->add(_stdoutFd.get(), common::core::io::IEventIO::E_IN);
	DEBUG(_logger, "spawn: registered stdout fd=" + common::core::utils::toString(_stdoutFd.get()) + " E_IN");

	_cgiStartTime = std::time(NULL);
	_spawned = true;
	INFO(_logger, "spawn: pid=" + common::core::utils::toString(_pid)
		+ " script=" + scriptPath
		+ " interpreter=" + interpreter);
}

/**
 * @brief [TODO:description]
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
 * @brief [TODO:description]
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
 * @brief [TODO:description]
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
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param mux [TODO:parameter]
 */
void CGIHandler::writeChunkToCGI(handler::RequestHandler &requestHandler, t_ioMultiplexer mux)
{
	const t_raw &buf = requestHandler.getBufferRequest();
	const t_Headers &headers = requestHandler.getRequest().getHeaders();
	t_Headers::const_iterator clIt = headers.find("content-length");
	if (clIt == headers.end() || clIt->second.empty())
	{
		DEBUG(_logger, "writeChunkToCGI: no Content-Length, closing stdin");
		mux->remove(_stdinFd.get());
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
			mux->remove(_stdinFd.get());
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
			mux->remove(_stdinFd.get());
			_stdinFd.reset();
			_bodySentDone = true;
		}
		return ;
	}
	ERROR(_logger, "writeChunkToCGI: write failed, terminating CGI");
	mux->remove(_stdinFd.get());
	_stdinFd.reset();
	throw client::HTTPError(502);
}

/**
 * @brief [TODO:description]
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

/**
 * @brief [TODO:description]
 *
 * @param requestHandler [TODO:parameter]
 * @param mux [TODO:parameter]
 * @param execFlags [TODO:parameter]
 */
void CGIHandler::driveIO(handler::RequestHandler &requestHandler,
		t_ioMultiplexer mux,
		int execFlags)
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
		common::core::io::IEventIO::e_Event ev = mux->getEvents(_stdinFd.get());
		if (ev & common::core::io::IEventIO::E_OUT)
		{
			DEBUG(_logger, "driveIO: stdin ready -> write");
			writeChunkToCGI(requestHandler, mux);
		}
	}

	if (_stdoutFd.valid())
	{
		common::core::io::IEventIO::e_Event ev = mux->getEvents(_stdoutFd.get());
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
 * @brief [TODO:description]
 *
 * @param mux [TODO:parameter]
 */
void CGIHandler::reset(t_ioMultiplexer mux)
{
	if (_stdinFd.valid())
	{
		try { mux->remove(_stdinFd.get()); } catch (...) {}
		_stdinFd.reset();
	}
	if (_stdoutFd.valid())
	{
		try { mux->remove(_stdoutFd.get()); } catch (...) {}
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

} // !handler
} // !webserv
