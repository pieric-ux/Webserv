// TODO: Don't forget to add 42 header !

/**
 * @file vchar.cpp
 * @brief
 */

#include <cctype>

namespace abnf {
namespace core {

/**
 * @brief 
 *
 * @param c 
 * @return 
 */
bool	isVChar(char c)
{
	return (std::isgraph(c));
}

} // !core
} // !abnf
