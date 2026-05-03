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
#include <webserv/config/method.hpp>

#include <cctype>

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

	DEBUG(_logger, "buildEnv method=" + config::methodToStr(request.getMethod())
		+ " script=" + scriptPath
		+ " interpreter=" + interpreter
		+ " query=\"" + request.getQuery() + "\"");

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
}

} // !handler
} // !webserv
