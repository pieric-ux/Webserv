// TODO: Don't forget to add 42 header !

/**
 * @file Abnf.hpp
 * @brief
 */

#ifndef ABNF_HPP
#define ABNF_HPP

#include "Loader.hpp"

namespace abnf
{

/**
 * @class Abnf
 * @brief 
 *
 */
class Abnf
{
	public:
		static Abnf &getAbnf();
		loader::Loader	&getLoader();

	private:
		Abnf();
		~Abnf();

		Abnf(const Abnf &rhs);
		Abnf &operator=(const Abnf &rhs);

		loader::Loader	&loader;

};

} // !abnf

#endif // !ABNF_HPP
