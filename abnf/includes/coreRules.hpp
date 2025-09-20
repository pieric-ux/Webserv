// TODO: Don't forget to add 42 header !

/**
 * @file coreRules.hpp
 * @brief
 */

#ifndef CORERULES_HPP
#define CORERULES_HPP

#include <cstddef>

namespace abnf
{
namespace core
{

	bool	isAlpha(char c);
	bool	isBit(char c);
	bool	isChar(unsigned char c);
	bool	isCR(char c);
	bool	isCRLF(const char *str, std::size_t &pos);
	bool	isCTL(char c);
	bool	isDigit(char c);
	bool	isDQuote(char c);
	bool	isHexdig(char c);
	bool	isHTab(char c);
	bool	isLF(char c);
	bool	isLWSP(const char *str, std::size_t &pos);
	bool	isOctet(char c);
	bool	isSP(char c);
	bool	isVChar(char c);
	bool	isWSP(char c);

} // !namespace core
} // !namespace abnf

#endif // !CORERULES_HPP
