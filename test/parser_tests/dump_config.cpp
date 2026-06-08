/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dump_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Temporary test: dump all parsed config to verify extraction
// Build: make && c++ -std=c++98 -I includes -I libs/parser/includes -I libs/logger/includes -I libs/common/includes test/parser_tests/dump_config.cpp -L. -L libs/parser -L libs/logger -L libs/common -labnf -llog42 -lcommon objs/*.o -o test_dump -Wl,-force_load,objs/main.o 2>/dev/null || true
// Alternatively, just link against webserv objects minus main

#include <webserv/config/method.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/config/Listen.hpp>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/Return.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/status/StatusCode.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace webserv;

static void dumpLocation(const config::LocationConfig &loc, const std::string &indent)
{
	std::cout << indent << "uri: " << loc.getUri() << "\n";
	std::cout << indent << "modifier: " << (loc.getModifier() == config::EXACT ? "=" : (loc.getModifier() == config::PREFIX_PRIORITY ? "^~" : "prefix")) << "\n";
	std::cout << indent << "root: " << loc.getRoot() << "\n";
	std::cout << indent << "autoindex: " << (loc.getAutoindex() ? "on" : "off") << "\n";

	t_Index idx = loc.getIndex();
	if (!idx.empty()) {
		std::cout << indent << "index:";
		for (size_t i = 0; i < idx.size(); ++i)
			std::cout << " " << idx[i];
		std::cout << "\n";
	}

	t_AllowedMethods am = loc.getAllowedMethods();
	if (!am.empty()) {
		std::cout << indent << "allowed_methods:";
		for (t_AllowedMethods::const_iterator it = am.begin(); it != am.end(); ++it)
			std::cout << " " << config::methodToStr(*it);
		std::cout << "\n";
	}

	std::cout << indent << "client_max_body_size: " << loc.getClientMaxBodySize() << "\n";
	std::cout << indent << "keepalive_timeout: " << loc.getKeepAliveTimeout() << "\n";
	std::cout << indent << "default_type: " << loc.getDefaultType() << "\n";

	t_ErrorPages ep = loc.getErrorPage();
	for (size_t i = 0; i < ep.size(); ++i)
		std::cout << indent << "error_page: (defined)\n";

	std::cout << indent << "enable_cgi: " << (loc.isEnableCGI() ? "on" : "off") << "\n";

	t_CgiExtensions ce = loc.getCgiExtensions();
	if (!ce.empty()) {
		std::cout << indent << "cgi_extensions:";
		for (t_CgiExtensions::const_iterator it = ce.begin(); it != ce.end(); ++it)
			std::cout << " " << it->first << "=" << it->second;
		std::cout << "\n";
	}

	std::cout << indent << "create_full_put_path: " << (loc.getCreateFullPutPath() ? "on" : "off") << "\n";

	std::ostringstream oss;
	oss << std::oct << loc.getDavAccess();
	std::cout << indent << "dav_access: 0" << oss.str() << "\n";

	t_DavMethods dm = loc.getDavMethods();
	if (!dm.empty()) {
		std::cout << indent << "dav_methods:";
		for (t_DavMethods::const_iterator it = dm.begin(); it != dm.end(); ++it)
			std::cout << " " << config::methodStr(*it);
		std::cout << "\n";
	}

	if (!loc.getDavPutPath().empty())
		std::cout << indent << "dav_put_path: " << loc.getDavPutPath() << "\n";

	config::Return redir = loc.getRedirect();
	if (redir.statusCode.getCode() != 0)
		std::cout << indent << "return: " << redir.statusCode.getCode() << " " << redir.url << "\n";

	t_MimeTypes types = loc.getTypes();
	if (!types.empty()) {
		std::cout << indent << "types:";
		for (t_MimeTypes::const_iterator it = types.begin(); it != types.end(); ++it)
			std::cout << " " << it->first << "=" << it->second;
		std::cout << "\n";
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config.conf>\n";
		return 1;
	}

	// Read config file
	std::ifstream file(argv[1]);
	if (!file.is_open()) {
		std::cerr << "Cannot open: " << argv[1] << "\n";
		return 1;
	}
	std::string content((std::istreambuf_iterator<char>(file)),
	                     std::istreambuf_iterator<char>());
	t_raw buffer(content.begin(), content.end());

	// Parse
	parser::Parser p;
	try {
		p.parseConfig(buffer);
	} catch (const std::exception &e) {
		std::cerr << "Parse error: " << e.what() << "\n";
		return 1;
	}

	config::HTTPConfig &cfg = config::HTTPConfig::getInstance();

	// Dump server configs
	t_ServerConfigs servers = cfg.getServerConfigs();
	std::cout << "=== " << servers.size() << " server(s) ===\n\n";

	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "--- Server " << i << " ---\n";
		const config::ServerConfig &srv = servers[i];

		t_Listen listens = srv.getListen();
		for (size_t l = 0; l < listens.size(); ++l) {
			std::cout << "  listen: " << listens[l].address << ":" << listens[l].port;
			if (listens[l].defaultServer) std::cout << " default_server";
			if (listens[l].reuseport) std::cout << " reuseport";
			if (listens[l].doBind) std::cout << " bind";
			if (listens[l].so_keepalive) std::cout << " so_keepalive=on";
			if (listens[l].backlog != 0) std::cout << " backlog=" << listens[l].backlog;
			std::cout << "\n";
		}

		t_Servernames names = srv.getServerName();
		if (!names.empty()) {
			std::cout << "  server_name:";
			for (size_t n = 0; n < names.size(); ++n)
				std::cout << " " << names[n];
			std::cout << "\n";
		}

		std::cout << "  root: " << srv.getRoot() << "\n";
		std::cout << "  client_max_body_size: " << srv.getClientMaxBodySize() << "\n";
		std::cout << "  keepalive_timeout: " << srv.getKeepAliveTimeout() << "\n";
		std::cout << "  default_type: " << srv.getDefaultType() << "\n";
		std::cout << "  enable_cgi: " << (srv.getEnableCGI() ? "on" : "off") << "\n";
		std::cout << "  create_full_put_path: " << (srv.getCreateFullPutPath() ? "on" : "off") << "\n";

		std::ostringstream oss;
		oss << std::oct << srv.getDavAccess();
		std::cout << "  dav_access: 0" << oss.str() << "\n";

		t_DavMethods dm = srv.getDavMethods();
		std::cout << "  dav_methods:";
		for (size_t d = 0; d < dm.size(); ++d)
			std::cout << " " << methodStr(dm[d]);
		std::cout << "\n";

		if (!srv.getDavPutPath().empty())
			std::cout << "  dav_put_path: " << srv.getDavPutPath() << "\n";

		t_CgiExtensions ce = srv.getCgiExtensions();
		if (!ce.empty()) {
			std::cout << "  cgi_extensions:";
			for (t_CgiExtensions::const_iterator it = ce.begin(); it != ce.end(); ++it)
				std::cout << " " << it->first << "=" << it->second;
			std::cout << "\n";
		}

		t_ErrorPages ep = srv.getErrorPage();
		for (size_t e = 0; e < ep.size(); ++e)
			std::cout << "  error_page: (defined)\n";

		t_MimeTypes types = srv.getTypes();
		if (!types.empty()) {
			std::cout << "  types:";
			for (t_MimeTypes::const_iterator it = types.begin(); it != types.end(); ++it)
				std::cout << " " << it->first << "=" << it->second;
			std::cout << "\n";
		}

		t_LocationConfigs locs = srv.getLocationConfigs();
		std::cout << "  locations: " << locs.size() << "\n";
		for (size_t j = 0; j < locs.size(); ++j) {
			std::cout << "\n    [location " << j << "]\n";
			dumpLocation(locs[j], "    ");
		}
		std::cout << "\n";
	}

	return 0;
}
