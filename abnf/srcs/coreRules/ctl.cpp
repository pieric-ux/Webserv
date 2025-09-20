// TODO: Don't forget to add 42 header !

/**
 * @file ctl.cpp
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
bool	isCTL(char c)
{
	return (std::iscntrl(c));
}

} // !core
} // !abnf
