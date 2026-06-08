/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeadersRegistry.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HEADERS_HTTPHEADERSREGISTRY_HPP
#define WEBSERV_HEADERS_HTTPHEADERSREGISTRY_HPP

/**
* @file HTTPHeadersRegistry.hpp
* @brief Declares the singleton registry of known HTTP header fields.
*/

#include <string>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/types.hpp>
namespace webserv
{
namespace HTTPheaders
{

/**
 * @class HTTPHeadersRegistry
 * @brief Singleton registry mapping canonical HTTP header names to their
 *        HTTPHeader descriptors, enabling case-insensitive lookup of standard
 *        fields from the IANA HTTP field name registry.
 *
 * @link https://www.iana.org/assignments/http-fields/http-fields.xhtml
 */
class HTTPHeadersRegistry
{
	public:
		static HTTPHeadersRegistry &getInstance();

		t_Logger			getLogger() const;

		HTTPHeader			getHeader(const std::string name) const;

	private:
		t_Logger			_logger;
		t_HeadersRegistry	_headers;

		HTTPHeadersRegistry();
		~HTTPHeadersRegistry();
		HTTPHeadersRegistry(const HTTPHeadersRegistry &rhs);
		HTTPHeadersRegistry &operator=(const HTTPHeadersRegistry &rhs);
};

} // !HTTPheaders
} // !webserv

#endif // !WEBSERV_HEADERS_HTTPHEADERSREGISTRY_HPP
