// TODO: Don't forget to add 42 header !

/**
 * @file char.cpp
 * @brief
 */

namespace abnf {
namespace core {

/**
 * @brief 
 *
 * @param c 
 * @return 
 */
bool	isChar(unsigned char c)
{
	return (c >= 0x01 && c <= 0x7F);
}

} // !core
} // !abnf
