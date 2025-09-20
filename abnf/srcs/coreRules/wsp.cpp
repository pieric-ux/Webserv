// TODO: Don't forget to add 42 header !

/**
 * @file wsp.cpp
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
bool	isWSP(char c)
{
	return (isblank(c));
}

} // !core
} // !abnf
