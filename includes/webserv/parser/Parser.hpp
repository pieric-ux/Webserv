// TODO: don't forget header

#ifndef WEBSERV_PARSER_PARSER_HPP
#define WEBSERV_PARSER_PARSER_HPP

/**
 * @file Parser.hpp
 * @brief [TODO:description]
 */

#include <vector>
#include <string>
#include <abnf/Abnf.hpp>
#include <webserv/headers/HTTPHeader.hpp>
#include <webserv/config/HTTPConfig.hpp>

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

		int						getFlags() const;
		void					setFlags(const int flags);
		bool					isIPv6Address(const std::string &address);
		HTTPConfig				parseConfig(const std::string &buffer);
		void					parseRequestLine(const std::string &bufferRequest);
		std::vector<HTTPHeader>	parseHeaders(const std::string &bufferRequest);

	private:
		static abnf::Abnf		_abnf;
		e_ParserFlags			_flags;
};

} // !parser
} // !webserv

#endif // !WEBSERV_PARSER_PARSER_HPP
