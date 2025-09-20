// TODO: Don't forget to add 42 header !

/**
 * @file stringUtils.cpp
 * @brief
 */

#include <cctype>
#include <string>

namespace utils
{

/**
 * @brief 
 *
 * @param str 
 * @return 
 */
std::string trim(const std::string &str)
{
	std::string::const_iterator	it = str.begin();
	while (it != str.end() && isblank(static_cast<unsigned char>(*it)))
		it++;

	if (it == str.end())
		return "";

	std::string::const_reverse_iterator rit = str.rbegin();
	while (rit != str.rend() && isblank(static_cast<unsigned char>(*rit)))
		rit++;

	return (std::string (it, rit.base()));
}

} // !utils
