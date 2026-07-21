/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_LOGGING_HPP
#define WEBSERV_LOGGING_HPP

#include <log42/Log42.hpp>

#ifndef WEBSERV_LOG_DIR
# define WEBSERV_LOG_DIR "logs"
#endif

# define LOG_FMT "[%(asctime)] - %(name) - %(levelname) - %(funcName)(): %(message) - file: %(filename) at line: %(lineno)"
#if __APPLE__
# define LOG_DATE_FMT "%Y-%m-%d %H:%M:%S,%%(msecs) %Z"
#elif __linux__
# define LOG_DATE_FMT "%Y-%m-%d %H:%M:%S,%(msecs) %Z"
#endif

#endif // !WEBSERV_LOGGING_HPP
