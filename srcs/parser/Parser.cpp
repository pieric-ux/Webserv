// TODO: don't forget header

/**
 * @file Parser.cpp
 * @brief [TODO:description]
 */
#include <webserv/parser/Parser.hpp>

namespace webserv
{
namespace parser
{

abnf::Abnf &Parser::_abnf = abnf::Abnf::getInstance();
config::HTTPConfig &Parser::_config = config::HTTPConfig::getInstance();

/**
 * @brief [TODO:description]
 */
Parser::Parser() : _flags(static_cast<e_ParserFlags>(0))
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.parser.parser");
    _logger->setLevel(log42::logRecord::DEBUG);
    INFO(_logger, "Parser instance created");
}

/**
 * @brief [TODO:description]
 */
Parser::~Parser() {}

/**
 * @brief [TODO:description]
 */
Parser::Parser(const Parser &rhs) : _logger(rhs._logger), _flags(rhs._flags) {}

/**
 * @brief [TODO:description]
 */
Parser &Parser::operator=(const Parser &rhs)
{
	if (this != &rhs)
	{
		_flags = rhs._flags;
		_logger = rhs._logger;
	}
	return (*this);
}

int Parser::getFlags() const{ return _flags; }

/**
 * @brief [TODO:description]
 */
t_Logger	Parser::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.parser.parser");
}

/**
 * @brief [TODO:description]
 */
void Parser::setFlags(const int flags) 
{ 
	_flags = static_cast<e_ParserFlags>(flags); 
}

/**
 * @brief [TODO:description]
 */
t_SubRules Parser::extractDirectives(const std::string &parentRule,
                                     const std::string &input,
                                     const std::string &directive,
                                     size_t depth) const
{
	t_SubRules result;
	_abnf.extractSubRulesVectorWithDepth(parentRule, "ServerConfig", input,
	                                     directive, result, depth, 0);
	return result;
}

/**
 * @brief [TODO:description]
 */
std::string Parser::extractValue(const std::string &directive,
                                 const std::string &subRule,
                                 const std::string &input) const
{
	std::string value;
	_abnf.extractSubRule(directive, "ServerConfig", input, subRule, value);
	return value;
}

/**
 * @brief [TODO:description]
 */
t_SubRules Parser::extractValues(const std::string &directive,
                                 const std::string &subRule,
                                 const std::string &input) const
{
	t_SubRules values;
	_abnf.extractSubRulesVector(directive, "ServerConfig", input, subRule, values);
	return values;
}

/**
 * @brief [TODO:description]
 */
void Parser::ensureAtMostOne(const t_SubRules &dirs, const std::string &name) const
{
	if (dirs.size() > 1)
	{
		CRITICAL(_logger, "Multiple " + name + " directives found\n");
		throw std::runtime_error("Multiple " + name + " directives found\n");
	}
}


/**
 * @brief [TODO:description]
 */
t_keepAliveTimeout Parser::parseTimeoutValue(const std::string &str)
{
	std::string::size_type start = str.find_first_of("0123456789");
	std::string::size_type end = str.find_last_of("0123456789");
	if (start == std::string::npos || end == std::string::npos)
		return 0;
	std::string digits = str.substr(start, end - start + 1);
	unsigned long timeout = std::strtoul(digits.c_str(), NULL, 10);
	if (timeout > static_cast<unsigned long>(std::numeric_limits<t_keepAliveTimeout>::max()))
		throw std::runtime_error("Keepalive timeout value too large: " + digits + "\n");
	return static_cast<t_keepAliveTimeout>(timeout);
}

/**
 * @brief [TODO:description]
 */
t_Perms Parser::parseDavAccessValue(const std::string &val) const
{
	t_Perms davAccess = 0;
	t_SubRules entries = extractValues("dav-access-val", "dav-access-entry", val);
	for (size_t i = 0; i < entries.size(); ++i)
	{
		std::string who = extractValue("dav-access-entry", "dav-access-who", entries[i]);
		std::string permsStr = extractValue("dav-access-entry", "dav-access-perms", entries[i]);
		unsigned short perms = 0;
		for (size_t j = 0; j < permsStr.size(); ++j)
		{
			if (permsStr[j] == 'r') perms |= 4;
			else if (permsStr[j] == 'w') perms |= 2;
			else if (permsStr[j] == 'x') perms |= 1;
		}
		if (who == "user") davAccess |= (perms << 6);
		else if (who == "group") davAccess |= (perms << 3);
		else if (who == "all") davAccess |= perms;
	}
	return davAccess;
}

/**
 * @brief [TODO:description]
 */
t_DavMethods Parser::parseDavMethodsValue(const std::string &val) const
{
	t_SubRules methodStrs = extractValues("dav-methods-val", "dav-method", val);
	t_DavMethods methods;
	for (size_t i = 0; i < methodStrs.size(); ++i)
	{
		config::e_Method m;
		if (config::strToMethod(methodStrs[i], m))
			methods.insert(m);
	}
	return methods;
}

/**
 * @brief [TODO:description]
 */
t_ErrorPages Parser::parseErrorPageDirs(const t_SubRules &dirs) const
{
	t_ErrorPages errorPages;
	for (size_t i = 0; i < dirs.size(); ++i)
	{
		t_SubRules statusCodeStrs = extractValues("error-page-dir", "status-code", dirs[i]);
		std::string uriPath = extractValue("error-page-dir", "uri-path", dirs[i]);
		for (size_t j = 0; j < statusCodeStrs.size(); ++j)
		{
			unsigned short code = static_cast<unsigned short>(
				std::strtoul(statusCodeStrs[j].c_str(), NULL, 10));
			std::string resolvedPath = uriPath;
			size_t codeDigit = 0;
			for (size_t k = 0; k < resolvedPath.size(); ++k)
			{
				if (std::isdigit(resolvedPath[k]))
					++codeDigit;
				else if (resolvedPath[k] == 'x' && codeDigit < statusCodeStrs[j].size())
				{
					resolvedPath[k] = statusCodeStrs[j][codeDigit];
					++codeDigit;
				}
			}
			t_StatusCodes codes;
			codes.push_back(status::StatusCode(code, "", ""));
			errorPages.push_back(config::ErrorPage(codes, resolvedPath));
			std::ostringstream epOss;
			epOss << "Error page: " << code << " -> " << resolvedPath;
			DEBUG(_logger, epOss.str());
		}
	}
	return errorPages;
}

/**
 * @brief [TODO:description]
 */
t_MimeTypes Parser::parseTypesBlockValue(const std::string &str) const
{
	t_SubRules typeMappings = extractValues("types-block", "type-mapping", str);
	t_MimeTypes types;
	for (size_t i = 0; i < typeMappings.size(); ++i)
	{
		std::string mimeType = extractValue("type-mapping", "mime-type", typeMappings[i]);
		t_SubRules fileExts = extractValues("type-mapping", "file-ext", typeMappings[i]);
		for (size_t j = 0; j < fileExts.size(); ++j)
			types[fileExts[j]] = mimeType;
	}
	return types;
}

/**
 * @brief [TODO:description]
 */
t_CgiExtensions Parser::parseCgiExtensionDirs(const t_SubRules &extDirs,
                                               const t_SubRules &extBlocks) const
{
	t_CgiExtensions exts;
	for (size_t i = 0; i < extDirs.size(); ++i)
	{
		std::string ext = extractValue("cgi-extension-dir", "cgi-ext", extDirs[i]);
		std::string path = extractValue("cgi-extension-dir", "file-path", extDirs[i]);
		exts[ext] = path;
	}
	if (!extBlocks.empty())
	{
		t_SubRules mappings = extractValues("cgi-extensions-block", "cgi-mapping", extBlocks[0]);
		for (size_t i = 0; i < mappings.size(); ++i)
		{
			std::string path = extractValue("cgi-mapping", "file-path", mappings[i]);
			std::string ext = extractValue("cgi-mapping", "cgi-ext", mappings[i]);
			exts[ext] = path;
		}
	}
	return exts;
}

/**
 * @brief [TODO:description]
 */
void Parser::checkConfigABNF(const t_raw &buffer)
{
	INFO(_logger, "Checking config ABNF...");
	_buffer = buffer;
	std::string buffer_str(_buffer.begin(), _buffer.end());
	if (!_abnf.match("config", "ServerConfig", buffer_str))
	{
		CRITICAL(_logger, "Failed to match config");
		throw std::runtime_error("Failed to match config");
	}
	INFO(_logger, "Successfully matched config");
}

/**
 * @brief [TODO:description]
 */
void Parser::parseConfig(const t_raw &buffer)
{
	INFO(_logger, "Starting to parse config...");
	checkConfigABNF(buffer);
	std::string bufferStr(_buffer.begin(), _buffer.end());

	// HTTP-level common dir
	parseCommonDirectives("config", bufferStr, 3, _config);

	// io-multiplexer-dir
	{
		t_SubRules ioDirs = extractDirectives("config", bufferStr, "io-multiplexer-dir", 3);
		ensureAtMostOne(ioDirs, "io-multiplexer-dir");
		if (!ioDirs.empty())
		{
			std::string ioVal = extractValue("io-multiplexer-dir", "io-multiplexer-val", ioDirs[0]);
			_config.setIOMultiplexer(ioVal);
		}
		DEBUG(_logger, "IO multiplexer: " + _config.getIOMultiplexer());
	}

	std::ostringstream httpOss;
	httpOss << "Client max body size: " << _config.getClientMaxBodySize() << " bytes";
	DEBUG(_logger, httpOss.str());
	DEBUG(_logger, "Default type: " + _config.getDefaultType());
	httpOss.str(""); httpOss << "Keepalive timeout: " << _config.getKeepAliveTimeout() << "s";
	DEBUG(_logger, httpOss.str());
	DEBUG(_logger, "Root: " + _config.getRoot());
	DEBUG(_logger, std::string("Enable CGI: ") + (_config.getEnableCGI() ? "on" : "off"));
	httpOss.str(""); httpOss << "DAV access: 0" << std::oct << _config.getDavAccess() << std::dec;
	DEBUG(_logger, httpOss.str());

	// Server blocks
	t_SubRules serverBlocks = extractDirectives("config", bufferStr, "server-block", 3);
	if (serverBlocks.empty())
	{
		CRITICAL(_logger, "No server blocks found in config\n");
		throw std::runtime_error("No server blocks found in config\n");
	}

	t_ServerConfigs serverConfigs;
	for (size_t i = 0; i < serverBlocks.size(); ++i)
	{
		config::ServerConfig srv;
		applyParentDefaults(_config, srv);
		parseCommonDirectives("server-block", serverBlocks[i], 2, srv);
		parseListenDirectives(serverBlocks[i], srv);
		parseServerNameDirectives(serverBlocks[i], srv);

		t_SubRules locBlocks = extractDirectives("server-block", serverBlocks[i],
		                                         "location-block", 2);
		t_LocationConfigs locs;
		for (size_t j = 0; j < locBlocks.size(); ++j)
			parseLocationBlock(locBlocks[j], locs, srv);
		srv.setLocationConfigs(locs);
		serverConfigs.push_back(srv);
	}
	_config.setServerConfigs(serverConfigs);

	std::ostringstream oss;
	oss << "Parsed " << serverConfigs.size() << " server block(s)";
	INFO(_logger, oss.str());
}

/**
 * @brief [TODO:description]
 */
void Parser::parseListenDirectives(const std::string &serverBlockStr,
                                   config::ServerConfig &config)
{
	t_SubRules listenDirs = extractDirectives("server-block", serverBlockStr, "listen-dir", 2);
	if (listenDirs.empty())
		return;

	t_Listen listenList;
	for (size_t l = 0; l < listenDirs.size(); ++l)
	{
		config::Listen listen;
		std::string portStr = extractValue("listen-dir", "port", listenDirs[l]);
		if (!portStr.empty())
			listen.port = portStr.c_str();

		std::string portOnlyStr;
		bool isPortOnly = _abnf.extractSubRule("listen-dir", "ServerConfig",
			listenDirs[l], "port-only", portOnlyStr) && !portOnlyStr.empty();
		if (!isPortOnly)
		{
			std::string ipv4Str;
			if (_abnf.extractSubRule("listen-dir", "ServerConfig", listenDirs[l],
				"IPv4address", ipv4Str) && !ipv4Str.empty())
				listen.address = ipv4Str;
			else
			{
				std::string ipv6Str;
				if (_abnf.extractSubRule("listen-dir", "ServerConfig", listenDirs[l],
					"IPv6address", ipv6Str) && !ipv6Str.empty())
					listen.address = ipv6Str;
				else
				{
					std::string hostnameStr;
					if (_abnf.extractSubRule("listen-dir", "ServerConfig", listenDirs[l],
						"hostname", hostnameStr) && !hostnameStr.empty())
						listen.address = hostnameStr;
				}
			}
		}

		t_SubRules listenOpts = extractValues("listen-dir", "listen-option", listenDirs[l]);
		std::ostringstream oss;
			oss << "[server-block] Parsing listen directive: " << (listen.address.empty() ? "" : listen.address) << ":" << (listen.port.empty() ? "" : listen.port);
			if (!listenOpts.empty())
			{
				oss << " with options:";
				for (size_t o = 0; o < listenOpts.size(); ++o)
					oss << " " << listenOpts[o];
			}
			DEBUG(_logger, oss.str());
		for (size_t o = 0; o < listenOpts.size(); ++o)
		{
			if (listenOpts[o] == "default_server")
				listen.defaultServer = true;
			else if (listenOpts[o] == "reuseport")
				listen.reuseport = true;
			else if (listenOpts[o] == "ipv6only=on")
				listen.ipv6only = true;
			else if (listenOpts[o] == "ipv6only=off")
				listen.ipv6only = false;
			else if (listenOpts[o] == "so_keepalive=on")
				listen.so_keepalive = true;
			else if (listenOpts[o] == "so_keepalive=off")
				listen.so_keepalive = false;
			else if (listenOpts[o].compare(0, 8, "backlog=") == 0)
				listen.backlog = static_cast<int>(std::strtoul(listenOpts[o].substr(8).c_str(), NULL, 10));
			else if (listenOpts[o].compare(0, 7, "rcvbuf=") == 0)
				listen.rcvbuf = static_cast<std::size_t>(parseMultiplier<int>(listenOpts[o].substr(7).c_str()));
			else if (listenOpts[o].compare(0, 7, "sndbuf=") == 0)
				listen.sndbuf = static_cast<std::size_t>(parseMultiplier<int>(listenOpts[o].substr(7).c_str()));
		}
		listenList.push_back(listen);
		std::ostringstream lOss;
		lOss << "[server-block] Listen: " << listen.address << ":" << listen.port;
		if (listen.defaultServer) lOss << " default_server";
		if (listen.reuseport) lOss << " reuseport";
		if (listen.ipv6only) lOss << " ipv6only=on";
		else lOss << " ipv6only=off";
		if (listen.so_keepalive) lOss << " so_keepalive=on";
		else lOss << " so_keepalive=off";
		if (listen.backlog >= 0) lOss << " backlog=" << listen.backlog;
		if (listen.rcvbuf >= 0) lOss << " rcvbuf=" << listen.rcvbuf;
		if (listen.sndbuf >= 0) lOss << " sndbuf=" << listen.sndbuf;
		DEBUG(_logger, lOss.str());
	}
	config.setListen(listenList);
}

/**
 * @brief [TODO:description]
 */
void Parser::parseServerNameDirectives(const std::string &serverBlockStr,
                                       config::ServerConfig &config)
{
	t_SubRules nameDirs = extractDirectives("server-block", serverBlockStr,
	                                        "server-name-dir", 2);
	if (nameDirs.empty())
		return;
	t_Servernames names;
	for (size_t i = 0; i < nameDirs.size(); ++i)
	{
		t_SubRules tokens = extractValues("server-name-dir", "server-name-token", nameDirs[i]);
		for (size_t j = 0; j < tokens.size(); ++j)
			names.push_back(tokens[j]);
	}
	config.setServerName(names);
	std::ostringstream snOss;
	snOss << "[server-block] Server names:";
	for (size_t i = 0; i < names.size(); ++i)
		snOss << " " << names[i];
	DEBUG(_logger, snOss.str());
}

/**
 * @brief [TODO:description]
 */
void Parser::parseLocationUri(const std::string &locationBlockStr,
                              config::LocationConfig &config)
{
	std::string uriPath;
	_abnf.extractSubRuleWithDepth("location-block", "ServerConfig", locationBlockStr,
	                              "uri-path", uriPath, 1, 0);
	config.setUri(uriPath);

	std::string modifierStr;
	if (_abnf.extractSubRuleWithDepth("location-block", "ServerConfig", locationBlockStr,
	                                  "modifier", modifierStr, 1, 0) && !modifierStr.empty())
	{
		if (modifierStr == "=")
			config.setModifier(config::EXACT);
		else if (modifierStr == "^~")
			config.setModifier(config::PREFIX_PRIORITY);
		else
			config.setModifier(config::PREFIX);
	}
	else
		config.setModifier(config::PREFIX);

	std::string modStr = (config.getModifier() == config::EXACT ? "= " :
	                      (config.getModifier() == config::PREFIX_PRIORITY ? "^~ " : ""));
	DEBUG(_logger, "[location-block] Location: " + modStr + config.getUri());
}

/**
 * @brief [TODO:description]
 */
void Parser::parseAutoindexDirective(const std::string &locationBlockStr,
                                     config::LocationConfig &config)
{
	t_SubRules dirs = extractDirectives("location-block", locationBlockStr, "autoindex-dir", 2);
	ensureAtMostOne(dirs, "autoindex-dir");
	if (!dirs.empty())
	{
		config.setAutoindex(extractValue("autoindex-dir", "on-off", dirs[0]) == "on");
		DEBUG(_logger, std::string("[location-block] Autoindex: ") + (config.getAutoindex() ? "on" : "off"));
	}
}

/**
 * @brief [TODO:description]
 */
void Parser::parseIndexDirective(const std::string &locationBlockStr,
                                 config::LocationConfig &config)
{
	t_SubRules dirs = extractDirectives("location-block", locationBlockStr, "index-dir", 2);
	ensureAtMostOne(dirs, "index-dir");
	if (!dirs.empty())
	{
		t_SubRules filenames = extractValues("index-dir", "filename", dirs[0]);
		t_Index indexFiles;
		for (size_t i = 0; i < filenames.size(); ++i)
			indexFiles.push_back(filenames[i]);
		config.setIndex(indexFiles);
		std::ostringstream idxOss;
		idxOss << "[location-block] Index:";
		for (size_t i = 0; i < indexFiles.size(); ++i)
			idxOss << " " << indexFiles[i];
		DEBUG(_logger, idxOss.str());
	}
}

/**
 * @brief [TODO:description]
 */
void Parser::parseAllowedMethodsDirective(const std::string &locationBlockStr,
                                          config::LocationConfig &config)
{
	t_SubRules dirs = extractDirectives("location-block", locationBlockStr,
	                                    "allowed-methods-dir", 2);
	ensureAtMostOne(dirs, "allowed-methods-dir");
	if (!dirs.empty())
	{
		t_SubRules methodStrs = extractValues("allowed-methods-dir", "method", dirs[0]);
		t_AllowedMethods methods;
		for (size_t i = 0; i < methodStrs.size(); ++i)
		{
			config::e_Method m;
			if (config::strToMethod(methodStrs[i], m))
				methods.insert(m);
		}
		config.setAllowedMethods(methods);
		std::ostringstream amOss;
		amOss << "[location-block] Allowed methods:";
		t_AllowedMethods::const_iterator it = methods.begin();
		for (; it != methods.end(); ++it)
			amOss << " " << config::methodToStr(*it);
		DEBUG(_logger, amOss.str());
	}
}

/**
 * @brief [TODO:description]
 */
void Parser::parseReturnDirective(const std::string &locationBlockStr,
                                  config::LocationConfig &config)
{
	t_SubRules dirs = extractDirectives("location-block", locationBlockStr, "return-dir", 2);
	ensureAtMostOne(dirs, "return-dir");
	if (!dirs.empty())
	{
		std::string statusCodeStr = extractValue("return-dir", "status-code", dirs[0]);
		unsigned short code = static_cast<unsigned short>(
			std::strtoul(statusCodeStr.c_str(), NULL, 10));
		std::string returnTarget;
		_abnf.extractSubRule("return-dir", "ServerConfig", dirs[0], "return-target", returnTarget);
		config::Return redirect;
		redirect.statusCode = status::StatusCode(code, "", "");
		redirect.url = returnTarget;
		config.setRedirect(redirect);
		std::ostringstream retOss;
		retOss << "[location-block] Return: " << code;
		if (!returnTarget.empty()) retOss << " " << returnTarget;
		DEBUG(_logger, retOss.str());
	}
}

/**
 * @brief Parse the HTTP request-line into the given Request.
 */
void Parser::parseRequestLine(const std::string &line, client::Request &request)
{
	if (!_abnf.match("request-line", "HTTP", line))
	{
		INFO(_logger, "400: invalid request-line: " + line.substr(0, 80));
		throw client::HTTPError(400);
	}

	std::string methodStr;
	std::string target;
	std::string version;

	if (!_abnf.extractSubRule("request-line", "HTTP", line, "method", methodStr)
		|| !_abnf.extractSubRule("request-line", "HTTP", line, "request-target", target)
		|| !_abnf.extractSubRule("request-line", "HTTP", line, "HTTP-version", version))
	{
		INFO(_logger, "400: failed to extract method/target/version from request-line");
		throw client::HTTPError(400);
	}

	if (target.size() > 8192)
	{
		std::ostringstream oss;
		oss << "414: URI too long (" << target.size() << " bytes)";
		INFO(_logger, oss.str());
		throw client::HTTPError(414);
	}

	if (version != "HTTP/1.1")
	{
		INFO(_logger, "505: unsupported HTTP version: " + version);
		throw client::HTTPError(505);
	}

	config::e_Method method;
	if (!config::strToMethod(methodStr, method))
	{
		INFO(_logger, "501: unimplemented method: " + methodStr);
		throw client::HTTPError(501);
	}

	std::string authority;
	std::string path;
	std::string query;

	if (_abnf.match("origin-form", "HTTP", target))
	{
		_abnf.extractSubRule("origin-form", "HTTP", target, "absolute-path", path);
		_abnf.extractSubRule("origin-form", "HTTP", target, "query", query);
	}
	else if (_abnf.match("absolute-form", "HTTP", target))
	{
		std::string scheme;
		_abnf.extractSubRule("absolute-form", "HTTP", target, "scheme", scheme);
		if (scheme != "http" && scheme != "https")
		{
			INFO(_logger, "400: unsupported scheme in absolute-form: " + scheme);
			throw client::HTTPError(400);
		}

		_abnf.extractSubRule("absolute-form", "HTTP", target, "authority", authority);
		_abnf.extractSubRule("absolute-form", "HTTP", target, "path-abempty", path);
		_abnf.extractSubRule("absolute-form", "HTTP", target, "query", query);
	}
	else if (_abnf.match("authority-form", "HTTP", target))
	{
		INFO(_logger, "400: authority-form not supported");
		throw client::HTTPError(400);
	}
	else if (_abnf.match("asterisk-form", "HTTP", target))
	{
		INFO(_logger, "400: asterisk-form not supported");
		throw client::HTTPError(400);
	}

	request.setMethod(method);
	request.setRequestTarget(target);
	request.setAuthority(authority);
	request.setPath(common::core::utils::urlDecode(path));
	request.setQuery(query);
	request.setHttpVersion(version);

	std::ostringstream rlOss;
	rlOss << "Request-line: " << methodStr << " " << path;
	if (!query.empty()) rlOss << "?" << request.getQuery();
	if (!authority.empty()) rlOss << " (authority: " << request.getAuthority() << ")";
	if (!path.empty()) rlOss << " path: " << common::core::utils::urlDecode(request.getPath());
	rlOss << " " << request.getHttpVersion();
	DEBUG(_logger, rlOss.str());
}

/**
 * @brief Parse the headers section (between request-line and CRLF CRLF).
 *
 */
void Parser::parseHeaders(const std::string &headersBlock, client::Request &request)
{
	t_Headers headers;
	std::string::size_type pos = 0;

	while (pos < headersBlock.size())
	{
		std::string::size_type eol = headersBlock.find("\r\n", pos);
		if (eol == std::string::npos)
		{
			INFO(_logger, "400: missing CRLF in headers block");
			throw client::HTTPError(400);
		}
		if (eol == pos)
			break;
		std::string line = headersBlock.substr(pos, eol - pos);
		pos = eol + 2;

		if (!_abnf.match("field-line", "HTTP", line))
		{
			INFO(_logger, "400: invalid header field-line: " + line.substr(0, 80));
			throw client::HTTPError(400);
		}

		std::string name;
		std::string value;
		if (!_abnf.extractSubRule("field-line", "HTTP", line, "field-name", name)
			|| !_abnf.extractSubRule("field-line", "HTTP", line, "field-value", value))
		{
			INFO(_logger, "400: failed to extract field-name/field-value from: " + line.substr(0, 80));
			throw client::HTTPError(400);
		}

		value = common::core::utils::toLower(common::core::utils::trim(value));
		name = common::core::utils::toLower(name);

		if (name == "host")
		{
			if (getFlags() & E_PARS_HOST)
			{
				INFO(_logger, "400: multiple Host headers found");
				throw client::HTTPError(400);
			}
			if (!_abnf.match("Host", "HTTP", value))
			{
				INFO(_logger, "400: invalid Host header format: " + value);
				throw client::HTTPError(400);
			}
			if (!request.getAuthority().empty())
				value = request.getAuthority();

			DEBUG(_logger, "Parsed Host header: " + value);
			
			setFlags(getFlags() | E_PARS_HOST);
		}
		else if (name == "connection")
		{
			if (!_abnf.match("Connection", "HTTP", common::core::utils::toLower(value)))
			{
				INFO(_logger, "400: Invalid Connection header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Connection header: " + value);
			setFlags(getFlags() | E_PARS_CONNECTION);
		}
		else if (name == "content-length")
		{
			if (!_abnf.match("Content-Length", "HTTP", value))
			{
				INFO(_logger, "400: Invalid Content-Length header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Content-Length header: " + value);
			if (getFlags() & E_PARS_CONTENT_LENGTH)
			{
				INFO(_logger, "400: Multiple Content-Length headers found");
				throw client::HTTPError(400);
			}
			setFlags(getFlags() | E_PARS_CONTENT_LENGTH);
		}
		else if (name == "content-type")
		{
			if (!_abnf.match("Content-Type", "HTTP", value))
			{
				INFO(_logger, "400: Invalid Content-Type header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Content-Type header: " + value);
			if (getFlags() & E_PARS_CONTENT_TYPE)
			{
				INFO(_logger, "400: Multiple Content-Type headers found");
				throw client::HTTPError(400);
			}
			setFlags(getFlags() | E_PARS_CONTENT_TYPE);
		}
		else if (name == "content-encoding")
		{
			if (!_abnf.match("Content-Encoding", "HTTP", common::core::utils::toLower(value)))
			{
				INFO(_logger, "400: Invalid Content-Encoding header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Content-Encoding header: " + value);
			setFlags(getFlags() | E_PARS_CONTENT_ENCODING);
		}
		else if (name == "expect")
		{
			if (!_abnf.match("Expect", "HTTP", common::core::utils::toLower(value)))
			{
				INFO(_logger, "400: Invalid Expect header format");
				throw client::HTTPError(400);
			}
			if (value != "100-continue")
			{
				INFO(_logger, "417: Unsupported Expect header value: " + value);
				throw client::HTTPError(417);
			}
			else
				DEBUG(_logger, "Parsed Expect header: " + value);
			setFlags(getFlags() | E_PARS_EXPECT);
		}
		else if (name =="last-modified")
		{
			if (!_abnf.match("Last-Modified", "HTTP", value))
			{
				INFO(_logger, "400: Invalid Last-Modified header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Last-Modified header: " + value);
		}
		else if (name == "date")
		{
			if (!_abnf.match("Date", "HTTP", value))
			{
				INFO(_logger, "400: Invalid Date header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Date header: " + value);
		}
		else if (name == "max-forwards")
		{
			if (!_abnf.match("Max-Forwards", "HTTP", value))
			{
				INFO(_logger, "400: Invalid Max-Forwards header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Max-Forwards header: " + value);
		}
		else if (name == "via")
		{
			if (!_abnf.match("Via", "HTTP", value))
			{
				INFO(_logger, "400: Invalid Via header format");
				throw client::HTTPError(400);
			}
			else
				DEBUG(_logger, "Parsed Via header: " + value);
		}
		if (name == "transfer-encoding")
		{
			if (!_abnf.match("Transfer-Encoding", "HTTP", common::core::utils::toLower(value)))
			{
				INFO(_logger, "400: invalid Transfer-Encoding header format: " + value);
				throw client::HTTPError(400);
			}
			INFO(_logger, "501: Transfer-Encoding not supported");
			throw client::HTTPError(501);
		}
			
		if (name == "range")
		{
			if (!_abnf.match("Range", "HTTP", value))
			{
				INFO(_logger, "400: invalid Range header format: " + value);
				throw client::HTTPError(400);
			}
			INFO(_logger, "501: Range not supported");
			throw client::HTTPError(501);
		}

		HTTPheaders::HTTPHeader header = HTTPheaders::HTTPHeadersRegistry::getInstance().getHeader(name);
		header.setValue(value);
		headers[common::core::utils::toLower(name)].push_back(header);
	}
	request.setHeaders(headers);

	setFlags(getFlags() | E_PARS_CLRF);

	std::ostringstream hdrsOss;
	hdrsOss << "Parsed " << headers.size() << " header(s)";
	DEBUG(_logger, hdrsOss.str());
}

} // !parser
} // !webserv
