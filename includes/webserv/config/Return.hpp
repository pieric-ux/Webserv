/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Return.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CONFIG_RETURN_HPP
#define WEBSERV_CONFIG_RETURN_HPP

/**
 * @file Return.hpp
 * @brief Defines the Return struct, holding the status code and URL of an nginx-style return directive.
 */

#include <string>
#include <webserv/status/StatusCode.hpp>

namespace webserv
{
namespace config
{

struct Return
{
	status::StatusCode	statusCode;
	std::string			url;
};

} // !config
} // !webserv

#endif // !WEBSERV_CONFIG_RETURN_HPP
