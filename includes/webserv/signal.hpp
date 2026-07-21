/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SIGNAL_HPP
#define WEBSERV_SIGNAL_HPP

/**
 * @file signal.hpp
 * @brief Declares the translation-unit-local, async-signal-safe flag set by the
 * signal handler to record the last received signal and request shutdown.
 */

#include <csignal>

namespace
{
	volatile std::sig_atomic_t	g_SignalStatus = 0;
}

#endif // !WEBSERV_SIGNAL_HPP
