/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusCodeRegistry.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_STATUS_STATUSCODEREGISTRY_HPP
#define WEBSERV_STATUS_STATUSCODEREGISTRY_HPP

/**
* @file StatusCodeRegistry.hpp
* @brief Declares the StatusCodeRegistry singleton that maps HTTP status codes to their StatusCode entries.
*/

#include <map>
#include <webserv/status/StatusCode.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace status
{

/**
 * @class StatusCodeRegistry
 * @brief Singleton registry holding the full set of HTTP status codes, each with its reason phrase and description, looked up by numeric code.
 *
 * @link https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
 */
class StatusCodeRegistry
{
	public:
		static StatusCodeRegistry	&getInstance();

		t_Logger					getLogger() const;

		StatusCode					getStatusCode(const unsigned short code) const;


	private:
		t_Logger					_logger;
		t_StatusCodesRegistry		_statusCodes;

		StatusCodeRegistry();
		~StatusCodeRegistry();
		StatusCodeRegistry(const StatusCodeRegistry &rhs);
		StatusCodeRegistry &operator=(const StatusCodeRegistry &rhs);
};

} // !status
} // !webserv

#endif // !WEBSERV_STATUS_STATUSCODEREGISTRY_HPP
