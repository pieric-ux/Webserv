/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusCode.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_STATUS_STATUSCODE_HPP
#define WEBSERV_STATUS_STATUSCODE_HPP

/**
* @file StatusCode.hpp
* @brief Declares the StatusCode value object representing an HTTP status code, its reason phrase and a human-readable description.
*/

#include <string>
#include <webserv/types.hpp>

namespace webserv
{
namespace status
{

/**
 * @class StatusCode
 * @brief Immutable value type bundling an HTTP status code with its reason-phrase message and description, exposing read-only accessors.
 */
class StatusCode
{
	public:
		StatusCode();
		StatusCode(const unsigned short code, const std::string msg, const std::string description);
		~StatusCode();

		StatusCode(const StatusCode &rhs);
		StatusCode &operator=(const StatusCode &rhs);

		static t_Logger			getLogger();

		int						getCode() const;
		const std::string		&getMessage() const;
		const std::string		&getDescription() const;

	private:
		t_Logger				_logger;
		unsigned short			_code;
		std::string				_msg;
		std::string				_description;
};

} // !status
} // !webserv

#endif // !WEBSERV_STATUS_STATUSCODE_HPP
