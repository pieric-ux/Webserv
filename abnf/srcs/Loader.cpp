// TODO: Don't forget to add 42 header !

/**
 * @file Loader.cpp
 * @brief
 */

#include "dirUtils.hpp"
#include "fileUtils.hpp"
#include "stringUtils.hpp"
#include "Loader.hpp"
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>

namespace abnf
{
namespace loader 
{

/**
 * @brief 
 */
Loader::Loader() {}

/**
 * @brief 
 */
Loader::~Loader() {}

/**
 * @brief 
 *
 * @return 
 */
Loader	&Loader::getLoader()
{
	static Loader	instance;

	return (instance);
}

/**
 * @brief 
 */
void	Loader::loadDirectory()
{
	Loader::loadDirectory("assets");
}

/**
 * @brief 
 *
 * @param dirname 
 */
void	Loader::loadDirectory(const std::string &dirname)
{
	utils::Directory dir(dirname);

	for (utils::DirectoryIterator it = dir.begin(); it != dir.end(); ++it)
	{
		struct dirent *entry = *it;
		if (entry->d_type == DT_REG && utils::hasExtension(entry->d_name, ".abnf"))
		{
			std::string filename = dirname + "/" + entry->d_name;
			try
			{
				std::string key = utils::filenameNoExt(entry->d_name, ".abnf");
				GrammarMap rules;
				this->loadFile(filename, rules);
				this->files[key] = rules;
			}
			catch (std::ios_base::failure &e)
			{
				throw std::runtime_error("Unable to open file " + filename + ": " + strerror(errno));
			}
		}
	}
}

/**
 * @brief 
 *
 * @param string 
 */
void	Loader::loadFile(const std::string &filename, GrammarMap &rules)
{
	std::ifstream	ifs;

	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	ifs.open(filename.c_str());

	this->parseRules(ifs, rules);
}

/**
 * @brief 
 *
 * @param ifs 
 */
void	Loader::parseRules(std::istream &ifs, GrammarMap &rules)
{
	std::string line;

	ifs.exceptions(std::ifstream::badbit);
	while (std::getline(ifs, line))
	{
		if (line.empty() || line[0] == ';')
			continue ;

		std::size_t pos = line.find('=');
		if (pos == std::string::npos)
			continue ;
		std::string	key = utils::trim(line.substr(0, pos));
		std::string value = utils::trim(line.substr(pos + 1));

		// std::cout << "[" << key << "] = [" << value << "]" << std::endl; // TODO: delete
		rules[key] = value;
	}
}

} // !loader
} // !abnf
