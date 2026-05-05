// TODO: don't forget header

#ifndef WEBSERV_HANDLER_CGIHANDLER_HPP
#define WEBSERV_HANDLER_CGIHANDLER_HPP

/**
 * @file CGIHandler.hpp
 * @brief [TODO:description]
 */

#include <ctime>
#include <vector>
#include <string>
#include <sys/socket.h>

#include <log42/Log42.hpp>
#include <common/common.hpp>

#include <webserv/types.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/client/Response.hpp>
#include <webserv/config/LocationConfig.hpp>

namespace webserv
{
namespace handler
{

class RequestHandler;
class ResponseHandler;

class CGIHandler
{
	public:
		CGIHandler(const t_ioMultiplexer &ioMultiplexer, sockaddr_storage clientAddr);
		~CGIHandler();

		static t_Logger								getLogger();

		void										spawn(const client::Request &request,
														const config::LocationConfig &locationConfig);
		void										driveIO(handler::RequestHandler &requestHandler, int execFlags);
		void										parse(client::Response &response);
		void										pushBody(handler::ResponseHandler &responseHandler);
		void										reset();

		t_ioMultiplexer								getIoMultiplexer() const;
		sockaddr_storage							getClientAddr() const;
		bool										hasFds() const;
		int											getStdinFd() const;
		int											getStdoutFd() const;
		bool										isSpawned() const;
		bool										isReaped() const;
		bool										isParsed() const;
		bool										isPushed() const;

	private:
		t_Logger									_logger;
		t_ioMultiplexer								_ioMultiplexer;
		sockaddr_storage							_clientAddr;
		pid_t										_pid;
		common::core::raii::UniqueFd				_stdinFd;
		common::core::raii::UniqueFd				_stdoutFd;
		std::time_t									_cgiStartTime;
		std::vector<std::string>					_envBuilder;
		common::core::raii::UniquePtr<char *[]>		_envp;
		t_raw										_cgiBuffer;
		t_raw										_cgiResponseBody;
		std::size_t									_bodySent;
		int											_exitStatus;
		bool										_spawned;
		bool										_bodySentDone;
		bool										_eof;
		bool										_reaped;
		bool										_parsed;
		bool										_pushed;

		std::string									headerToCgiName(const std::string &name);
		void										setNonblock(int fd);
		std::pair<std::size_t, std::size_t>			findHeaderEnd(const t_raw &v);

		void										buildEnv(const client::Request &request,
														const std::string &interpreter,
														const std::string &scriptPath);
		void										addEnv(const std::string &key, const std::string &value);
		char										**finalizeEnvp();
		void										killAndReap();
		void										tryReap();
		void										checkTimeout();
		void										writeChunkToCGI(handler::RequestHandler &requestHandler);
		void										readChunkFromCGI();

		CGIHandler(const CGIHandler &rhs);
		CGIHandler &operator=(const CGIHandler &rhs);
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_CGIHANDLER_HPP
