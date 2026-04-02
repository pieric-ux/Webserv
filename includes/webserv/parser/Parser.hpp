// TODO: don't forget header

#ifndef WEBSERV_PARSER_PARSER_HPP
#define WEBSERV_PARSER_PARSER_HPP

/**
 * @file Parser.hpp
 * @brief [TODO:description]
 */

 #include <cerrno>
#include <climits>
#include <vector>
#include <string>
#include <abnf/Abnf.hpp>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/config/HTTPConfig.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
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
	E_PARS_CLRF = 1 << 6
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
		bool						isIPv6Address(const std::string &address);
		void						parseConfig(const t_raw &buffer);
		void						parseRequestLine(const t_raw &bufferRequest);
		t_Headers					parseHeaders(const t_raw &bufferRequest);

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
		t_Perms						parseDavAccessValue(const std::string &val) const;
		t_DavMethods				parseDavMethodsValue(const std::string &val) const;
		t_ErrorPages				parseErrorPageDirs(const t_SubRules &dirs) const;
		t_MimeTypes					parseTypesBlockValue(const std::string &str) const;
		t_CgiExtensions				parseCgiExtensionDirs(const t_SubRules &extDirs,
										const t_SubRules &extBlocks) const;

		template <typename ConfigT>
		void						parseCommonDirectives(const std::string &parentRule,
										const std::string &input,
										size_t depth, ConfigT &config);
		template <typename ValueT>
		static ValueT				parseMultiplier(const std::string &val);

		void						parseListenDirectives(const std::string &serverBlockStr,
										config::ServerConfig &config);
		void						parseServerNameDirectives(const std::string &serverBlockStr,
										config::ServerConfig &config);
		void						parseLocationBlock(const std::string &locationBlockStr,
										t_LocationConfigs &locationConfigs);
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
};

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

	dirs = extractDirectives(parentRule, input, "create-full-put-path-dir", depth);
	ensureAtMostOne(dirs, "create-full-put-path-dir");
	if (!dirs.empty())
	{
		config.setCreateFullPutPath(extractValue("create-full-put-path-dir", "on-off", dirs[0]) == "on");
		DEBUG(_logger, ctx + "Create full PUT path: " + (config.getCreateFullPutPath() ? "on" : "off"));
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
		for (size_t i = 0; i < dm.size(); ++i)
			oss << ((dm[i] == config::PUT) ? "PUT" : "DELETE") << " ";
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
		DEBUG(_logger, ctx + "Enable CGI: " + (config.getEnableCGI() ? "on" : "off"));
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

} // !parser
} // !webserv

#endif // !WEBSERV_PARSER_PARSER_HPP
