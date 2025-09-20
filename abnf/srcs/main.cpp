// TODO: Don't forget to add 42 header !

#include "Abnf.hpp"
#include <iostream>

int	main(void)
{
	abnf::Abnf				&module = abnf::Abnf::getAbnf();
	abnf::loader::Loader	&ldr = module.getLoader();

	try
	{
		ldr.loadDirectory();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}
