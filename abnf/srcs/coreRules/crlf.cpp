// TODO: Don't forget to add 42 header !

/**
 * @file crlf.cpp
 * @brief
 */

#include <cstddef>

namespace abnf {
namespace core {

/**
 * @brief 
 *
 * @param str 
 * @param pos 
 * @return 
 */
bool	isCRLF(const char *str, std::size_t &pos)
{
	if (str[pos] == '\r' && str[pos + 1] == '\n')
	{
		pos += 2;
		return (true);
	}
	return (false);
}

} // !core
} // !abnf
