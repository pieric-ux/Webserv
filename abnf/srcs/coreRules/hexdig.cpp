// TODO: Don't forget to add 42 header !

/**
 * @file hexdig.cpp
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
bool	isHexdig(char c)
{
	return (std::isdigit(c) || (c >= 'A' && c <= 'F'));
}

} // !core
} // !abnf
