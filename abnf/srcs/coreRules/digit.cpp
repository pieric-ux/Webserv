// TODO: Don't forget to add 42 header !

/**
 * @file digit.cpp
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
bool	isDigit(char c)
{
	return (std::isdigit(c));
}

} // !namespace core
} // !namespace abnf
