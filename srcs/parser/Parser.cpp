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

/**
 * @brief [TODO:description]
 */
abnf::Abnf &Parser::_abnf = abnf::Abnf::getInstance();

/**
 * @brief [TODO:description]
 */
Parser::Parser() : _flags(static_cast<e_ParserFlags>(0))
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.parser.parser");
    _logger->setLevel(log42::logRecord::INFO);
}

/**
 * @brief [TODO:description]
 */
Parser::~Parser() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Parser::Parser(const Parser &rhs) : _logger(rhs._logger), _flags(rhs._flags) {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
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

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
int Parser::getFlags() const
{
	return _flags;
}

/**
 * @brief [TODO:description]
 *
 * @param flags [TODO:parameter]
 */
void Parser::setFlags(const int flags)
{
	_flags = static_cast<e_ParserFlags>(flags);
}

/**
 * @brief [TODO:description]
 *
 * @param address [TODO:parameter]
 * @return [TODO:return]
 */
bool Parser::isIPv6Address(const std::string &address)
{
	(void)address;
	return false;
}

/**
 * @brief [TODO:description]
 *
 * @param buffer [TODO:parameter]
 * @return [TODO:return]
 */
config::HTTPConfig& Parser::parseConfig(const std::string &buffer)
{
	(void)buffer;
	return config::HTTPConfig::getInstance();
}

/**
 * @brief [TODO:description]
 *
 * @param bufferRequest [TODO:parameter]
 */
void Parser::parseRequestLine(const std::string &bufferRequest)
{
	(void)bufferRequest;
}

/**
 * @brief [TODO:description]
 *
 * @param bufferRequest [TODO:parameter]
 * @return [TODO:return]
 */
std::vector<HTTPheaders::HTTPHeader> Parser::parseHeaders(const std::string &bufferRequest)
{
	(void)bufferRequest;
	return std::vector<HTTPheaders::HTTPHeader>();
}

} // !parser
} // !webserv
