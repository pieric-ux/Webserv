/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_PARSER_PARSER_HPP
#define WEBSERV_PARSER_PARSER_HPP

/**
 * @file Parser.hpp
 * @brief Declares the parser::Parser class that validates and parses the
 *        server configuration (via the ABNF grammar) and HTTP request-lines
 *        and header blocks into config and client::Request objects.
 */

#include <cerrno>
#include <string>
#include <sstream>
#include <cctype>
#include <limits>
#include <cstdlib>
#include <abnf/Abnf.hpp>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/config/Listen.hpp>
#include <webserv/config/ErrorPage.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/Return.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/headers/HTTPHeadersRegistry.hpp>
#include <webserv/status/StatusCode.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace parser
{

enum e_ParserFlags
{
	E_PARS_HOST = 1 << 0,
	E_PARS_CONNECTION = 1 << 1,
	E_PARS_CONTENT_LENGTH = 1 << 2,
	E_PARS_CONTENT_TYPE = 1 << 3,
	E_PARS_CONTENT_ENCODING = 1 << 4,
	E_PARS_EXPECT = 1 << 5,
	E_PARS_CLRF = 1 << 6,
	E_PARS_COOKIE = 1 << 7,
	E_PARS_TRANSFER_ENCODING = 1 << 8
};

class Parser
{
	public:
		Parser();
		~Parser();

		Parser(const Parser &rhs);
		Parser &operator=(const Parser &rhs);

		static t_Logger				getLogger();

		int							getFlags() const;
		void						setFlags(const int flags);
		void						parseConfig(const t_raw &buffer);
		void						parseRequestLine(const std::string &line, client::Request &request);
		void						parseHeaders(const std::string &headerBlock, client::Request &request);

	private:
		static config::HTTPConfig	&_config;
		static abnf::Abnf			&_abnf;
		t_Logger					_logger;
		e_ParserFlags				_flags;
		t_raw						_buffer;

		// ABNF extraction helpers
		void 						checkConfigABNF(const t_raw &buffer);
		t_SubRules					extractDirectives(const std::string &parentRule,
										const std::string &input,
										const std::string &directive,
										size_t depth) const;
		std::string					extractValue(const std::string &directive,
										const std::string &subRule,
										const std::string &input) const;
		t_SubRules					extractValues(const std::string &directive,
										const std::string &subRule,
										const std::string &input) const;

		void						ensureAtMostOne(const t_SubRules &dirs,
										const std::string &name) const;
		static t_keepAliveTimeout	parseTimeoutValue(const std::string &str);
		static t_sessionTTL			parseSessionTTLValue(const std::string &str);
		t_Perms						parseDavAccessValue(const std::string &val) const;
		t_DavMethods				parseDavMethodsValue(const std::string &val) const;
		t_ErrorPages				parseErrorPageDirs(const t_SubRules &dirs) const;
		t_MimeTypes					parseTypesBlockValue(const std::string &str) const;
		t_CgiExtensions				parseCgiExtensionDirs(const t_SubRules &extDirs,
										const t_SubRules &extBlocks) const;

		void						parseListenDirectives(const std::string &serverBlockStr,
										config::ServerConfig &config);
		void						parseServerNameDirectives(const std::string &serverBlockStr,
										config::ServerConfig &config);
		void						parseLocationUri(const std::string &locationBlockStr,
										config::LocationConfig &config);
		void						parseAutoindexDirective(const std::string &locationBlockStr,
										config::LocationConfig &config);
		void						parseIndexDirective(const std::string &locationBlockStr,
										config::LocationConfig &config);
		void						parseAllowedMethodsDirective(const std::string &locationBlockStr,
										config::LocationConfig &config);
		void						parseReturnDirective(const std::string &locationBlockStr,
										config::LocationConfig &config);

		template <typename ConfigT>
		void						parseCommonDirectives(const std::string &parentRule,
										const std::string &input,
										size_t depth, ConfigT &config);
	
		template <typename ParentT, typename ChildT>
		static void					applyParentDefaults(const ParentT &parent, ChildT &child);

		template <typename ParentT>
		void						parseLocationBlock(const std::string &locationBlockStr,
										t_LocationConfigs &locationConfigs,
										const ParentT &parent);

		template <typename ValueT>
		static ValueT				parseMultiplier(const std::string &val);
};

/**
 * @brief Parses a numeric string with an optional size suffix (K, M or G,
 *        case-insensitive) into a byte count, multiplying by 1024, 1024^2 or
 *        1024^3 respectively.
 *
 * @tparam ValueT Integral target type the parsed value is checked against and
 *         cast to.
 * @param val The string to parse (e.g. "10", "5M", "1G").
 * @return The parsed value cast to ValueT.
 * @throws std::runtime_error if the string is not a valid number or the result
 *         overflows ValueT.
 */
template <typename ValueT>
ValueT Parser::parseMultiplier(const std::string &val)
{
	unsigned long size = 0;
	const char *start = NULL;
	char *endptr = NULL;
	std::string numPart;
	errno = 0;

	switch (std::toupper(val[val.length() - 1]))
	{
		case 'G' :
			numPart = val.substr(0, val.length() - 1);
			start = numPart.c_str();
			size = std::strtoul(start, &endptr, 0) * 1024 * 1024 * 1024;
			break;
		case 'M' :
			numPart = val.substr(0, val.length() - 1);
			start = numPart.c_str();
			size = std::strtoul(start, &endptr, 0) * 1024 * 1024;
			break;
		case 'K' :
			numPart = val.substr(0, val.length() - 1);
			start = numPart.c_str();
			size = std::strtoul(start, &endptr, 0) * 1024;
			break;
		default :
			start = val.c_str();
			size = std::strtoul(start, &endptr, 0);
	}
		if (endptr == start || errno == ERANGE
		|| (size > static_cast<unsigned long>(std::numeric_limits<ValueT>::max())
			&& size != 0))
		throw std::runtime_error("Invalid numeric value: " + val + "\n");
	return static_cast<ValueT>(size);
}

/**
 * @brief Extracts and applies the directives shared across configuration scopes
 *        (client-max-body-size, dav-*, default-type, error-page, keepalive and
 *        session timeouts, root, types, CGI enable and extensions) onto the
 *        given config object.
 *
 * @tparam ConfigT Configuration type exposing the matching setters/getters
 *         (HTTPConfig, ServerConfig or LocationConfig).
 * @param parentRule Name of the enclosing ABNF rule scope being parsed.
 * @param input The raw text of the block to extract directives from.
 * @param depth Nesting depth passed to the directive extraction helper.
 * @param config The configuration object to populate.
 */
template <typename ConfigT>
void Parser::parseCommonDirectives(const std::string &parentRule,
                                   const std::string &input,
                                   size_t depth, ConfigT &config)
{
	t_SubRules dirs;
	std::string ctx = "[" + parentRule + "] ";

	dirs = extractDirectives(parentRule, input, "client-max-body-size-dir", depth);
	ensureAtMostOne(dirs, "client-max-body-size-dir");
	if (!dirs.empty())
	{
		config.setClientMaxBodySize(parseMultiplier<t_clientMaxBodySize>((
			extractValue("client-max-body-size-dir", "size-value", dirs[0]))));
		std::ostringstream oss;
		oss << ctx << "Client max body size: " << config.getClientMaxBodySize() << " bytes";
		DEBUG(_logger, oss.str());
	}

	dirs = extractDirectives(parentRule, input, "dav-put-path-dir", depth);
	ensureAtMostOne(dirs, "dav-put-path-dir");
	if (!dirs.empty())
	{
		config.setDavPutPath(extractValue("dav-put-path-dir", "file-path", dirs[0]));
		DEBUG(_logger, ctx + "DAV PUT path: " + config.getDavPutPath());
	}

	dirs = extractDirectives(parentRule, input, "dav-access-dir", depth);
	ensureAtMostOne(dirs, "dav-access-dir");
	if (!dirs.empty())
	{
		config.setDavAccess(parseDavAccessValue(
			extractValue("dav-access-dir", "dav-access-val", dirs[0])));
		std::ostringstream oss;
		oss << ctx << "DAV access: 0" << std::oct << config.getDavAccess() << std::dec;
		DEBUG(_logger, oss.str());
	}

	dirs = extractDirectives(parentRule, input, "dav-methods-dir", depth);
	ensureAtMostOne(dirs, "dav-methods-dir");
	if (!dirs.empty())
	{
		config.setDavMethods(parseDavMethodsValue(
			extractValue("dav-methods-dir", "dav-methods-val", dirs[0])));
		std::ostringstream oss;
		oss << ctx << "DAV methods: ";
		t_DavMethods dm = config.getDavMethods();
		for (t_DavMethods::iterator it = dm.begin(); it != dm.end(); ++it)
			oss << ((*it == config::PUT) ? "PUT" : "DELETE") << " ";
		DEBUG(_logger, oss.str());
	}

	dirs = extractDirectives(parentRule, input, "default-type-dir", depth);
	ensureAtMostOne(dirs, "default-type-dir");
	if (!dirs.empty())
	{
		config.setDefaultType(extractValue("default-type-dir", "mime-type", dirs[0]));
		DEBUG(_logger, ctx + "Default type: " + config.getDefaultType());
	}

	dirs = extractDirectives(parentRule, input, "error-page-dir", depth);
	if (!dirs.empty())
		config.setErrorPage(parseErrorPageDirs(dirs));

	// keepalive-timeout-dir
	dirs = extractDirectives(parentRule, input, "keepalive-timeout-dir", depth);
	ensureAtMostOne(dirs, "keepalive-timeout-dir");
	if (!dirs.empty())
	{
		config.setKeepAliveTimeout(parseTimeoutValue(dirs[0]));
		std::ostringstream oss;
		oss << ctx << "Keepalive timeout: " << config.getKeepAliveTimeout() << "s";
		DEBUG(_logger, oss.str());
	}

	// session-ttl-dir
	dirs = extractDirectives(parentRule, input, "session-ttl-dir", depth);
	ensureAtMostOne(dirs, "session-ttl-dir");
	if (!dirs.empty())
	{
		config.setSessionTTL(parseSessionTTLValue(extractValue("session-ttl-dir", "time-value", dirs[0])));
		std::ostringstream oss;
		oss << ctx << "Session TTL: " << config.getSessionTTL() << "s";
		DEBUG(_logger, oss.str());
	}

	// root-dir
	dirs = extractDirectives(parentRule, input, "root-dir", depth);
	ensureAtMostOne(dirs, "root-dir");
	if (!dirs.empty())
	{
		config.setRoot(extractValue("root-dir", "file-path", dirs[0]));
		DEBUG(_logger, ctx + "Root: " + config.getRoot());
	}

	// types-block
	dirs = extractDirectives(parentRule, input, "types-block", depth);
	ensureAtMostOne(dirs, "types-block");
	if (!dirs.empty())
	{
		config.setTypes(parseTypesBlockValue(dirs[0]));
		std::ostringstream oss;
		oss << ctx << "Types: ";
		t_MimeTypes types = config.getTypes();
		for (t_MimeTypes::const_iterator it = types.begin(); it != types.end(); ++it)
			oss << it->first << "=" << it->second << " ";
		DEBUG(_logger, oss.str());
	}

	// enable-cgi-dir
	dirs = extractDirectives(parentRule, input, "enable-cgi-dir", depth);
	ensureAtMostOne(dirs, "enable-cgi-dir");
	if (!dirs.empty())
	{
		config.setEnableCGI(extractValue("enable-cgi-dir", "on-off", dirs[0]) == "on");
		DEBUG(_logger, ctx + "Enable CGI: " + (config.isEnableCGI() ? "on" : "off"));
	}

	// cgi-extension-dir + cgi-extensions-block
	t_SubRules extDirs = extractDirectives(parentRule, input, "cgi-extension-dir", depth);
	t_SubRules extBlocks = extractDirectives(parentRule, input, "cgi-extensions-block", depth);
	ensureAtMostOne(extBlocks, "cgi-extensions-block");
	t_CgiExtensions exts = parseCgiExtensionDirs(extDirs, extBlocks);
	if (!exts.empty())
	{
		config.setCgiExtensions(exts);
		std::ostringstream oss;
		oss << ctx << "CGI extensions: ";
		for (t_CgiExtensions::const_iterator it = exts.begin(); it != exts.end(); ++it)
			oss << it->first << "=" << it->second << " ";
		DEBUG(_logger, oss.str());
	}
}

/**
 * @brief Copies the inheritable directive values from a parent configuration
 *        scope into a child scope, so the child starts with the parent's
 *        defaults before its own directives are parsed.
 *
 * @tparam ParentT Configuration type of the enclosing scope.
 * @tparam ChildT Configuration type of the nested scope.
 * @param parent The configuration to read default values from.
 * @param child The configuration to populate with the parent's values.
 */
template <typename ParentT, typename ChildT>
void Parser::applyParentDefaults(const ParentT &parent, ChildT &child)
{
	child.setClientMaxBodySize(parent.getClientMaxBodySize());
	child.setDavPutPath(parent.getDavPutPath());
	child.setDavAccess(parent.getDavAccess());
	child.setDavMethods(parent.getDavMethods());
	child.setDefaultType(parent.getDefaultType());
	child.setErrorPage(parent.getErrorPage());
	child.setKeepAliveTimeout(parent.getKeepAliveTimeout());
	child.setSessionTTL(parent.getSessionTTL());
	child.setRoot(parent.getRoot());
	child.setTypes(parent.getTypes());
	child.setEnableCGI(parent.isEnableCGI());
	child.setCgiExtensions(parent.getCgiExtensions());
}

/**
 * @brief Parses a single location block into a LocationConfig (inheriting the
 *        parent's defaults, then its URI, common, autoindex, index, allowed
 *        methods and return directives), appends it to the list, and recurses
 *        into any nested location blocks.
 *
 * @tparam ParentT Configuration type of the enclosing scope providing defaults.
 * @param locationBlockStr The raw text of the location block to parse.
 * @param locationConfigs The collection that parsed locations are appended to.
 * @param parent The enclosing configuration whose defaults are inherited.
 */
template <typename ParentT>
void Parser::parseLocationBlock(const std::string &locationBlockStr,
                                t_LocationConfigs &locationConfigs,
                                const ParentT &parent)
{
	config::LocationConfig loc;
	applyParentDefaults(parent, loc);
	parseLocationUri(locationBlockStr, loc);
	parseCommonDirectives("location-block", locationBlockStr, 2, loc);
	parseAutoindexDirective(locationBlockStr, loc);
	parseIndexDirective(locationBlockStr, loc);
	parseAllowedMethodsDirective(locationBlockStr, loc);
	parseReturnDirective(locationBlockStr, loc);
	locationConfigs.push_back(loc);

	t_SubRules nested = extractDirectives("location-block", locationBlockStr,
	                                      "location-block", 2);
	if (!nested.empty())
	{
		std::ostringstream nestOss;
		nestOss << "[location-block] " << nested.size() << " nested location(s) under " << loc.getUri();
		DEBUG(_logger, nestOss.str());
	}
	for (size_t n = 0; n < nested.size(); ++n)
		parseLocationBlock(nested[n], locationConfigs, loc);
}

} // !parser
} // !webserv

#endif // !WEBSERV_PARSER_PARSER_HPP
