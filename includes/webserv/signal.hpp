// TODO: don't forget header

#ifndef WEBSERV_SIGNAL_HPP
#define WEBSERV_SIGNAL_HPP

/**
 * @file signal.hpp
 * @brief [TODO:description]
 */

#include <csignal>

namespace
{
	volatile std::sig_atomic_t	g_SignalStatus = 0;
}

#endif // !WEBSERV_SIGNAL_HPP
