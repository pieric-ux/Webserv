// TODO: Don't forget to add 42 header !

/**
 * @file lwsp.cpp
 * @brief
 */

#include <cctype>
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
bool	isLWSP(const char *str, std::size_t &pos)
{
	size_t start = pos;
	while (true)
	{
		if (isblank(str[pos]))
			pos++;
		else if (str[pos] == '\r' && str[pos + 1] == '\n' && isblank(str[pos + 2]))
			pos += 3;
		else
			break ;
	}
	return (pos > start);
}

} // !core
} // !abnf
