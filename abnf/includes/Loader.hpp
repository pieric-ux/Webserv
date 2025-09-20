// TODO: Don't forget to add 42 header !

/**
 * @file Loader.hpp
 * @brief
 */

#ifndef LOADER_HPP
#define LOADER_HPP

#include <map>
#include <string>

namespace abnf
{
namespace loader
{

#define DIRNAME "assets"

class Loader
{
	public:
		typedef std::map<std::string, std::string>	GrammarMap;
		typedef std::map<std::string, GrammarMap>	GrammarFilesMap;

		static Loader &getLoader();

		void	loadDirectory();
		void	loadDirectory(const std::string &dirname);

	private:
		Loader();
		~Loader();

		Loader(const Loader &rhs);
		Loader &operator=(const Loader &rhs);

		void	loadFile(const std::string &filename, GrammarMap &rules);
		void	parseRules(std::istream &ifs, GrammarMap &rules);

		GrammarFilesMap	files;

};

} // !loader
} // !abnf

#endif // !LOADER_HPP
