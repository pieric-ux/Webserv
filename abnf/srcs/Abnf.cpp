// TODO: Don't forget to add 42 header !

/**
 * @file Abnf.cpp
 * @brief
 */

#include "Abnf.hpp"

namespace abnf 
{

/**
 * @brief 
 */
Abnf::Abnf() : loader(loader::Loader::getLoader()) {}

/**
 * @brief 
 */
Abnf::~Abnf() {}

/**
 * @brief 
 *
 * @return 
 */
Abnf	&Abnf::getAbnf()
{
	static Abnf instance;

	return (instance);
}

/**
 * @brief 
 *
 * @return 
 */
loader::Loader	&Abnf::getLoader()
{
	return (this->loader);
}

} // !abnf
