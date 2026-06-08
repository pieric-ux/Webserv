/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   method.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CONFIG_METHOD_HPP
#define WEBSERV_CONFIG_METHOD_HPP

#include <string>

namespace webserv
{
namespace config
{

enum e_Method
{
	GET,
	HEAD,
	POST,
	PUT,
	DELETE
};

inline std::string methodToStr(e_Method m)
{
	switch (m)
	{
		case GET:    return "GET";
		case HEAD:   return "HEAD";
		case POST:   return "POST";
		case PUT:    return "PUT";
		case DELETE: return "DELETE";
		default:     return "UNKNOWN";
	}
}

inline bool strToMethod(const std::string &s, e_Method &out)
{
	if (s == "GET")    { out = GET;    return true; }
	if (s == "HEAD")   { out = HEAD;   return true; }
	if (s == "POST")   { out = POST;   return true; }
	if (s == "PUT")    { out = PUT;    return true; }
	if (s == "DELETE") { out = DELETE; return true; }
	return false;
}

} //!config
} //!webserv

#endif //!WEBSERV_CONFIG_METHOD_HPP