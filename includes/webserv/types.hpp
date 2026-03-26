// TODO: don't forget header

#ifndef WEBSERV_TYPES_HPP
#define WEBSERV_TYPES_HPP

#include <list>
#include <vector>
#include <log42/Log42.hpp>
#include <common/common.hpp>
#include <common/core/io/IEventIO.hpp>

namespace webserv
{
namespace HTTPheaders { class HTTPHeader; }

/**
 * @typedef t_Logger
 * @brief Shared pointer to a logger instance.
 */
typedef common::core::raii::SharedPtr<log42::logger::Logger> 		t_Logger;

typedef common::core::raii::SharedPtr<common::core::io::IEventIO>	t_ioMultiplexer;

typedef std::vector<std::list<HTTPheaders::HTTPHeader> >			t_Headers;
	
} // !webserv

#endif // !WEBSERV_TYPES_HPP